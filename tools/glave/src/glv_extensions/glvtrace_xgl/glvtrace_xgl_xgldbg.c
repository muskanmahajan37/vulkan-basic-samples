/**************************************************************************
 *
 * Copyright 2014 Valve Software
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/
#include "glv_platform.h"
#include "glvtrace_xgl_xgldbg.h"
#include "glvtrace_xgl_xgldbg_structs.h"
#include "glvtrace_xgl_packet_id.h"
#include "glv_common.h"
#ifdef WIN32
#include "mhook/mhook-lib/mhook.h"
#endif

// ======================== Pointers to real functions ========================
static XGL_RESULT( XGLAPI * real_xglDbgSetValidationLevel)(
    XGL_DEVICE             device,
    XGL_VALIDATION_LEVEL   validationLevel) = xglDbgSetValidationLevel;

static XGL_RESULT( XGLAPI * real_xglDbgRegisterMsgCallback)(
    XGL_DBG_MSG_CALLBACK_FUNCTION pfnMsgCallback,
    XGL_VOID*                     pUserData) = xglDbgRegisterMsgCallback;

static XGL_RESULT( XGLAPI * real_xglDbgUnregisterMsgCallback)(
    XGL_DBG_MSG_CALLBACK_FUNCTION pfnMsgCallback) = xglDbgUnregisterMsgCallback;

static XGL_RESULT( XGLAPI * real_xglDbgSetMessageFilter)(
    XGL_DEVICE           device,
    XGL_INT              msgCode,
    XGL_DBG_MSG_FILTER   filter) = xglDbgSetMessageFilter;

static XGL_RESULT( XGLAPI * real_xglDbgSetObjectTag)(
    XGL_BASE_OBJECT object,
    XGL_SIZE        tagSize,
    const XGL_VOID* pTag) = xglDbgSetObjectTag;

static XGL_RESULT( XGLAPI * real_xglDbgSetGlobalOption)(
    XGL_DBG_GLOBAL_OPTION        dbgOption,
    XGL_SIZE                     dataSize,
    const XGL_VOID*              pData) = xglDbgSetGlobalOption;

static XGL_RESULT( XGLAPI * real_xglDbgSetDeviceOption)(
    XGL_DEVICE                   device,
    XGL_DBG_DEVICE_OPTION        dbgOption,
    XGL_SIZE                     dataSize,
    const XGL_VOID*              pData) = xglDbgSetDeviceOption;

static XGL_VOID( XGLAPI * real_xglCmdDbgMarkerBegin)(
    XGL_CMD_BUFFER  cmdBuffer,
    const XGL_CHAR* pMarker) = xglCmdDbgMarkerBegin;

static XGL_VOID( XGLAPI * real_xglCmdDbgMarkerEnd)(
    XGL_CMD_BUFFER  cmdBuffer) = xglCmdDbgMarkerEnd;

void AttachHooks_xgldbg()
{
    BOOL hookSuccess = TRUE;
#if defined(WIN32)
    Mhook_BeginMultiOperation(FALSE);
    if (real_xglDbgSetValidationLevel != NULL)
    {
        hookSuccess = Mhook_SetHook((PVOID*)&real_xglDbgSetValidationLevel, hooked_xglDbgSetValidationLevel);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglDbgRegisterMsgCallback, hooked_xglDbgRegisterMsgCallback);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglDbgUnregisterMsgCallback, hooked_xglDbgUnregisterMsgCallback);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglDbgSetMessageFilter, hooked_xglDbgSetMessageFilter);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglDbgSetObjectTag, hooked_xglDbgSetObjectTag);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglDbgSetGlobalOption, hooked_xglDbgSetGlobalOption);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglDbgSetDeviceOption, hooked_xglDbgSetDeviceOption);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglCmdDbgMarkerBegin, hooked_xglCmdDbgMarkerBegin);
        hookSuccess &= Mhook_SetHook((PVOID*)&real_xglCmdDbgMarkerEnd, hooked_xglCmdDbgMarkerEnd);
    }

    if (!hookSuccess)
    {
        glv_LogError("Failed to hook XGLDbg.");
    }

    Mhook_EndMultiOperation();
#elif defined(__linux__)
    hookSuccess = glv_platform_get_next_lib_sym((PVOID*)&real_xglDbgSetValidationLevel, "xglDbgSetValidationLevel");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglDbgRegisterMsgCallback, "xglDbgRegisterMsgCallback");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglDbgUnregisterMsgCallback, "xglDbgUnregisterMsgCallback");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglDbgSetMessageFilter, "xglDbgSetMessageFilter");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglDbgSetObjectTag, "xglDbgSetObjectTag");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglDbgSetGlobalOption,"xglDbgSetGlobalOption");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglDbgSetDeviceOption, "xglDbgSetDeviceOption");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglCmdDbgMarkerBegin, "xglCmdDbgMarkerBegin");
    hookSuccess &= glv_platform_get_next_lib_sym((PVOID*)&real_xglCmdDbgMarkerEnd, "xglCmdDbgMarkerEnd");
    if (!hookSuccess)
    {
        glv_LogError("Failed to hook XGLDbg.");
    }
#endif
}

void DetachHooks_xgldbg()
{
 
#ifdef WIN32
    BOOL unhookSuccess = TRUE;

    if (real_xglDbgSetValidationLevel != NULL)
    {
        unhookSuccess = Mhook_Unhook((PVOID*)&real_xglDbgSetValidationLevel);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglDbgRegisterMsgCallback);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglDbgUnregisterMsgCallback);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglDbgSetMessageFilter);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglDbgSetObjectTag);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglDbgSetGlobalOption);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglDbgSetDeviceOption);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglCmdDbgMarkerBegin);
        unhookSuccess &= Mhook_Unhook((PVOID*)&real_xglCmdDbgMarkerEnd);
    }
    
    if (!unhookSuccess)
    {
        glv_LogError("Failed to unhook XGLDbg.");
    }
#elif defined(__linux__)
    return;
#endif
}

GLVTRACER_EXPORT XGL_RESULT XGLAPI __HOOKED_xglDbgSetValidationLevel(
    XGL_DEVICE             device,
    XGL_VALIDATION_LEVEL   validationLevel)
{
    glv_trace_packet_header* pHeader;
    XGL_RESULT result;
    struct_xglDbgSetValidationLevel* pPacket;
    SEND_ENTRYPOINT_ID(xglDbgSetValidationLevel);
    CREATE_TRACE_PACKET(xglDbgSetValidationLevel, 0);
    result = real_xglDbgSetValidationLevel(device, validationLevel);
    pPacket = interpret_body_as_xglDbgSetValidationLevel(pHeader);
    pPacket->device = device;
    pPacket->validationLevel = validationLevel;
    pPacket->result = result;
    FINISH_TRACE_PACKET();
    return result;
}

GLVTRACER_EXPORT XGL_RESULT XGLAPI __HOOKED_xglDbgRegisterMsgCallback(
    XGL_DBG_MSG_CALLBACK_FUNCTION pfnMsgCallback,
    XGL_VOID*                     pUserData)
{
    glv_trace_packet_header* pHeader;
    XGL_RESULT result;
    struct_xglDbgRegisterMsgCallback* pPacket;
    SEND_ENTRYPOINT_ID(xglDbgRegisterMsgCallback);
    CREATE_TRACE_PACKET(xglDbgRegisterMsgCallback, 0);
    result = real_xglDbgRegisterMsgCallback(pfnMsgCallback, pUserData);
    pPacket = interpret_body_as_xglDbgRegisterMsgCallback(pHeader);
    pPacket->pfnMsgCallback = pfnMsgCallback;
    pPacket->pUserData = pUserData;
    pPacket->result = result;
    FINISH_TRACE_PACKET();
    return result;
}

GLVTRACER_EXPORT XGL_RESULT XGLAPI __HOOKED_xglDbgUnregisterMsgCallback(
    XGL_DBG_MSG_CALLBACK_FUNCTION pfnMsgCallback)
{
    glv_trace_packet_header* pHeader;
    XGL_RESULT result;
    struct_xglDbgUnregisterMsgCallback* pPacket;
    SEND_ENTRYPOINT_ID(xglDbgUnregisterMsgCallback);
    CREATE_TRACE_PACKET(xglDbgUnregisterMsgCallback, 0);
    result = real_xglDbgUnregisterMsgCallback(pfnMsgCallback);
    pPacket = interpret_body_as_xglDbgUnregisterMsgCallback(pHeader);
    pPacket->pfnMsgCallback = pfnMsgCallback;
    pPacket->result = result;
    FINISH_TRACE_PACKET();
    return result;
}

GLVTRACER_EXPORT XGL_RESULT XGLAPI __HOOKED_xglDbgSetMessageFilter(
    XGL_DEVICE           device,
    XGL_INT              msgCode,
    XGL_DBG_MSG_FILTER   filter)
{
    glv_trace_packet_header* pHeader;
    XGL_RESULT result;
    struct_xglDbgSetMessageFilter* pPacket;
    SEND_ENTRYPOINT_ID(xglDbgSetMessageFilter);
    CREATE_TRACE_PACKET(xglDbgSetMessageFilter, 0);
    result = real_xglDbgSetMessageFilter(device, msgCode, filter);
    pPacket = interpret_body_as_xglDbgSetMessageFilter(pHeader);
    pPacket->device = device;
    pPacket->msgCode = msgCode;
    pPacket->filter = filter;
    pPacket->result = result;
    FINISH_TRACE_PACKET();
    return result;
}

GLVTRACER_EXPORT XGL_RESULT XGLAPI __HOOKED_xglDbgSetObjectTag(
    XGL_BASE_OBJECT object,
    XGL_SIZE        tagSize,
    const XGL_VOID* pTag)
{
    glv_trace_packet_header* pHeader;
    XGL_RESULT result;
    struct_xglDbgSetObjectTag* pPacket;
    SEND_ENTRYPOINT_ID(xglDbgSetObjectTag);
    CREATE_TRACE_PACKET(xglDbgSetObjectTag, tagSize);
    result = real_xglDbgSetObjectTag(object, tagSize, pTag);
    pPacket = interpret_body_as_xglDbgSetObjectTag(pHeader);
    pPacket->object = object;
    pPacket->tagSize = tagSize;
    glv_add_buffer_to_trace_packet(pHeader, (void**)&(pPacket->pTag), tagSize, pTag);
    pPacket->result = result;
    glv_finalize_buffer_address(pHeader, (void**)&(pPacket->pTag));
    FINISH_TRACE_PACKET();
    return result;
}

GLVTRACER_EXPORT XGL_RESULT XGLAPI __HOOKED_xglDbgSetGlobalOption(
    XGL_DBG_GLOBAL_OPTION        dbgOption,
    XGL_SIZE                     dataSize,
    const XGL_VOID*              pData)
{
    glv_trace_packet_header* pHeader;
    XGL_RESULT result;
    struct_xglDbgSetGlobalOption* pPacket;
    SEND_ENTRYPOINT_ID(xglDbgSetGlobalOption);
    CREATE_TRACE_PACKET(xglDbgSetGlobalOption, dataSize);
    result = real_xglDbgSetGlobalOption(dbgOption, dataSize, pData);
    pPacket = interpret_body_as_xglDbgSetGlobalOption(pHeader);
    pPacket->dbgOption = dbgOption;
    pPacket->dataSize = dataSize;
    glv_add_buffer_to_trace_packet(pHeader, (void**)&(pPacket->pData), dataSize, pData);
    pPacket->result = result;
    glv_finalize_buffer_address(pHeader, (void**)&(pPacket->pData));
    FINISH_TRACE_PACKET();
    return result;
}

GLVTRACER_EXPORT XGL_RESULT XGLAPI __HOOKED_xglDbgSetDeviceOption(
    XGL_DEVICE                   device,
    XGL_DBG_DEVICE_OPTION        dbgOption,
    XGL_SIZE                     dataSize,
    const XGL_VOID*              pData)
{
    glv_trace_packet_header* pHeader;
    XGL_RESULT result;
    struct_xglDbgSetDeviceOption* pPacket;
    SEND_ENTRYPOINT_ID(xglDbgSetDeviceOption);
    CREATE_TRACE_PACKET(xglDbgSetDeviceOption, dataSize);
    result = real_xglDbgSetDeviceOption(device, dbgOption, dataSize, pData);
    pPacket = interpret_body_as_xglDbgSetDeviceOption(pHeader);
    pPacket->device = device;
    pPacket->dbgOption = dbgOption;
    pPacket->dataSize = dataSize;
    glv_add_buffer_to_trace_packet(pHeader, (void**)&(pPacket->pData), dataSize, pData);
    pPacket->result = result;
    glv_finalize_buffer_address(pHeader, (void**)&(pPacket->pData));
    FINISH_TRACE_PACKET();
    return result;
}

GLVTRACER_EXPORT XGL_VOID XGLAPI __HOOKED_xglCmdDbgMarkerBegin(
    XGL_CMD_BUFFER  cmdBuffer,
    const XGL_CHAR* pMarker)
{
    glv_trace_packet_header* pHeader;
    struct_xglCmdDbgMarkerBegin* pPacket;
    SEND_ENTRYPOINT_ID(xglCmdDbgMarkerBegin);
    CREATE_TRACE_PACKET(xglCmdDbgMarkerBegin, strlen((const char *)pMarker) + 1);
    real_xglCmdDbgMarkerBegin(cmdBuffer, pMarker);
    pPacket = interpret_body_as_xglCmdDbgMarkerBegin(pHeader);
    pPacket->cmdBuffer = cmdBuffer;
    glv_add_buffer_to_trace_packet(pHeader, (void**)&(pPacket->pMarker), strlen((const char *)pMarker) + 1, pMarker);
    glv_finalize_buffer_address(pHeader, (void**)&(pPacket->pMarker));
    FINISH_TRACE_PACKET();
}

GLVTRACER_EXPORT XGL_VOID XGLAPI __HOOKED_xglCmdDbgMarkerEnd(
    XGL_CMD_BUFFER  cmdBuffer)
{
    glv_trace_packet_header* pHeader;
    struct_xglCmdDbgMarkerEnd* pPacket;
    SEND_ENTRYPOINT_ID(xglCmdDbgMarkerEnd);
    CREATE_TRACE_PACKET(xglCmdDbgMarkerEnd, 0);
    real_xglCmdDbgMarkerEnd(cmdBuffer);
    pPacket = interpret_body_as_xglCmdDbgMarkerEnd(pHeader);
    pPacket->cmdBuffer = cmdBuffer;
    FINISH_TRACE_PACKET();
}
