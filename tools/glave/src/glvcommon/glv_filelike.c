/*
 * Copyright (c) 2013, NVIDIA CORPORATION. All rights reserved.
 * Copyright (c) 2014, Valve Software. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "glv_filelike.h"
#include "glv_common.h"
#include "glv_interconnect.h"
#include <assert.h>
#include <stdlib.h>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Checkpoint* glv_Checkpoint_create(const char* _str)
{
    Checkpoint* pCheckpoint = GLV_NEW(Checkpoint);
    pCheckpoint->mToken = _str;
    pCheckpoint->mTokenLength = strlen(_str) + 1;
    return pCheckpoint;
}

// ------------------------------------------------------------------------------------------------
void glv_Checkpoint_write(Checkpoint* pCheckpoint, FileLike* _out)
{
    glv_FileLike_Write(_out, pCheckpoint->mToken, pCheckpoint->mTokenLength);
}

// ------------------------------------------------------------------------------------------------
BOOL glv_Checkpoint_read(Checkpoint* pCheckpoint, FileLike* _in)
{
    if (pCheckpoint->mTokenLength < 64) {
        char buffer[64];
        glv_FileLike_Read(_in, buffer, pCheckpoint->mTokenLength);
        if (strcmp(buffer, pCheckpoint->mToken) != 0) {
            return FALSE;
        }
    } else {
        char* buffer = GLV_NEW_ARRAY(char, pCheckpoint->mTokenLength);
        glv_FileLike_Read(_in, buffer, pCheckpoint->mTokenLength);
        if (strcmp(buffer, pCheckpoint->mToken) != 0) {
            GLV_DELETE(buffer);
            return FALSE;
        }
        GLV_DELETE(buffer);
    }
    return TRUE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
FileLike* glv_FileLike_create_file(FILE* fp)
{
    FileLike* pFile = NULL;
    if (fp != NULL)
    {
        pFile = GLV_NEW(FileLike);
        pFile->mMode = File;
        pFile->mFile = fp;
        pFile->mMessageStream = NULL;
    }
    return pFile;
}

// ------------------------------------------------------------------------------------------------
FileLike* glv_FileLike_create_msg(MessageStream* _msgStream)
{
    FileLike* pFile = NULL;
    if (_msgStream != NULL)
    {
        pFile = GLV_NEW(FileLike);
        pFile->mMode = Socket;
        pFile->mFile = NULL;
        pFile->mMessageStream = _msgStream;
    }
    return pFile;
}

// ------------------------------------------------------------------------------------------------
size_t glv_FileLike_Read(FileLike* pFileLike, void* _bytes, size_t _len)
{
    size_t bytesInStream = 0;
    if (glv_FileLike_ReadRaw(pFileLike, &bytesInStream, sizeof(bytesInStream)) == FALSE)
        return 0;
    
    if (bytesInStream > 0) {
        assert(_len >= bytesInStream);
        if (glv_FileLike_ReadRaw(pFileLike, _bytes, min(_len, bytesInStream)) == FALSE)
            return 0;
    }

    return min(_len, bytesInStream);
}

// ------------------------------------------------------------------------------------------------
BOOL glv_FileLike_ReadRaw(FileLike* pFileLike, void* _bytes, size_t _len)
{
    BOOL result = TRUE;
    assert((pFileLike->mFile != 0) ^ (pFileLike->mMessageStream != 0));

    switch(pFileLike->mMode) {
    case File:
        {
            if (1 != fread(_bytes, _len, 1, pFileLike->mFile))
            {
                if (ferror(pFileLike->mFile) != 0)
                {
                    perror("fread error");
                }
                else if (feof(pFileLike->mFile) != 0)
                {
                    glv_LogWarn("Reached end of file\n");
                }
                result = FALSE;
            } 
            break;
        }
    case Socket:
        {
            result = glv_MessageStream_BlockingRecv(pFileLike->mMessageStream, _bytes, _len);
            break;
        }

        default: 
            assert(!"Invalid mode in FileLike_ReadRaw");
            result = FALSE;
    }
    return result;
}

void glv_FileLike_Write(FileLike* pFileLike, const void* _bytes, size_t _len)
{
    glv_FileLike_WriteRaw(pFileLike, &_len, sizeof(_len));
    if (_len) {
        glv_FileLike_WriteRaw(pFileLike, _bytes, _len);
    }
}

// ------------------------------------------------------------------------------------------------
BOOL glv_FileLike_WriteRaw(FileLike* pFile, const void* _bytes, size_t _len)
{
    BOOL result = TRUE;
    assert((pFile->mFile != 0) ^ (pFile->mMessageStream != 0));
    switch (pFile->mMode)
    {
        case File:
            if (1 != fwrite(_bytes, _len, 1, pFile->mFile))
            {
                result = FALSE;
            }
            break;
        case Socket:
            result = glv_MessageStream_Send(pFile->mMessageStream, _bytes, _len);
            break;
        default:
            assert(!"Invalid mode in FileLike_WriteRaw");
            result = FALSE;
            break;
    }
    return result;
}
