/**************************************************************************
 * Rutoken                                                                *
 * Copyright (c) 2020-2021, Aktiv-Soft JSC. All rights reserved.          *
 * Подробная информация:  http://www.rutoken.ru                           *
 *------------------------------------------------------------------------*
 * Данный файл содержит объявление констант для работы с Рутокен в UEFI   *
 * на языке C                                                             *
 *************************************************************************/
#pragma once

/*************************************************************************
 * Включение файлов:                                                     *
 *  - UefiLib.h - для доступа к определениям функции Print, макроса      *
 *    EFI_ERROR и кодов ошибок UEFI                                      *
 ************************************************************************/
#include <Library/UefiLib.h>

/**************************************************************************
 * Функция преобразования ошибки UEFI к строке                            *
 *************************************************************************/
static const CHAR16* StatusToStr(EFI_STATUS Status) {
    switch (Status) {
    case EFI_SUCCESS: return L"EFI_SUCCESS";
    case EFI_LOAD_ERROR: return L"EFI_LOAD_ERROR";
    case EFI_INVALID_PARAMETER: return L"EFI_INVALID_PARAMETER";
    case EFI_UNSUPPORTED: return L"EFI_UNSUPPORTED";
    case EFI_BAD_BUFFER_SIZE: return L"EFI_BAD_BUFFER_SIZE";
    case EFI_BUFFER_TOO_SMALL: return L"EFI_BUFFER_TOO_SMALL";
    case EFI_NOT_READY: return L"EFI_NOT_READY";
    case EFI_DEVICE_ERROR: return L"EFI_DEVICE_ERROR";
    case EFI_WRITE_PROTECTED: return L"EFI_WRITE_PROTECTED";
    case EFI_OUT_OF_RESOURCES: return L"EFI_OUT_OF_RESOURCES";
    case EFI_VOLUME_CORRUPTED: return L"EFI_VOLUME_CORRUPTED";
    case EFI_VOLUME_FULL: return L"EFI_VOLUME_FULL";
    case EFI_NO_MEDIA: return L"EFI_NO_MEDIA";
    case EFI_MEDIA_CHANGED: return L"EFI_MEDIA_CHANGED";
    case EFI_NOT_FOUND: return L"EFI_NOT_FOUND";
    case EFI_ACCESS_DENIED: return L"EFI_ACCESS_DENIED";
    case EFI_NO_RESPONSE: return L"EFI_NO_RESPONSE";
    case EFI_NO_MAPPING: return L"EFI_NO_MAPPING";
    case EFI_TIMEOUT: return L"EFI_TIMEOUT";
    case EFI_NOT_STARTED: return L"EFI_NOT_STARTED";
    case EFI_ALREADY_STARTED: return L"EFI_ALREADY_STARTED";
    case EFI_ABORTED: return L"EFI_ABORTED";
    case EFI_ICMP_ERROR: return L"EFI_ICMP_ERROR";
    case EFI_TFTP_ERROR: return L"EFI_TFTP_ERROR";
    case EFI_PROTOCOL_ERROR: return L"EFI_PROTOCOL_ERROR";
    case EFI_INCOMPATIBLE_VERSION: return L"EFI_INCOMPATIBLE_VERSION";
    case EFI_SECURITY_VIOLATION: return L"EFI_SECURITY_VIOLATION";
    case EFI_CRC_ERROR: return L"EFI_CRC_ERROR";
    case EFI_END_OF_MEDIA: return L"EFI_END_OF_MEDIA";
    case EFI_END_OF_FILE: return L"EFI_END_OF_FILE";
    case EFI_INVALID_LANGUAGE: return L"EFI_INVALID_LANGUAGE";
    case EFI_COMPROMISED_DATA: return L"EFI_COMPROMISED_DATA";
    case EFI_HTTP_ERROR: return L"EFI_HTTP_ERROR";
    case EFI_WARN_UNKNOWN_GLYPH: return L"EFI_WARN_UNKNOWN_GLYPH";
    case EFI_WARN_DELETE_FAILURE: return L"EFI_WARN_DELETE_FAILURE";
    case EFI_WARN_WRITE_FAILURE: return L"EFI_WARN_WRITE_FAILURE";
    case EFI_WARN_BUFFER_TOO_SMALL: return L"EFI_WARN_BUFFER_TOO_SMALL";
    case EFI_WARN_STALE_DATA: return L"EFI_WARN_STALE_DATA";
    case EFI_WARN_FILE_SYSTEM: return L"EFI_WARN_FILE_SYSTEM";
    default: return L"Unknown error";
    }
}

#if defined(_MSC_VER)
#define VA_ARGS(...) , __VA_ARGS__
#else
#define VA_ARGS(...) , ##__VA_ARGS__
#endif

#define MAX_LOG_STR_LEN 16

/**************************************************************************
 * Макрос вывода массива байт в шестнадцатеричном формате                 *
 *************************************************************************/
#define UEFI_LOG_XXD(buf, size, msg, ...)               \
    do {                                                \
        Print(msg VA_ARGS(__VA_ARGS__));                \
        for (int i = 0; i < size; ++i) {                \
            if (i % MAX_LOG_STR_LEN == 0) Print(L"\n"); \
            Print(L"%02X ", buf[i]);                    \
        }                                               \
        Print(L"\n");                                   \
    } while (0)

/**************************************************************************
 * Макрос проверки ошибки. Если произошла ошибка, то выводится            *
 * сообщение и осуществляется переход на заданную метку                   *
 *************************************************************************/
#define UEFI_CHECK_ERROR_AND_LOG(msg, status, errMsg, label) \
    do {                                                     \
        Print(L"%s", msg);                                   \
        if (EFI_ERROR(status)) {                             \
            Print(L" -> Failed\n%s\n", errMsg);              \
            goto label;                                      \
        } else {                                             \
            Print(L" -> OK\n");                              \
        }                                                    \
    } while (0)

/**************************************************************************
 * Макрос проверки ошибки при освобождении ресурсов. Если произошла       *
 * ошибка, то выводится сообщение и выставляется                          *
 * значение переменной errorCode                                          *
 *************************************************************************/
#define UEFI_CHECK_RELEASE_AND_LOG(msg, status, errMsg, errorCode) \
    do {                                                           \
        Print(L"%s", msg);                                         \
        if (EFI_ERROR(status)) {                                   \
            Print(L" -> Failed\n%s\n", errMsg);                    \
            errorCode = 1;                                         \
        } else {                                                   \
            Print(L" -> OK\n");                                    \
        }                                                          \
    } while (0)

/**************************************************************************
 * Макрос проверки возвращаемого значения APDU команды.                   *
 * Если произошла ошибка, то выводится сообщение и осуществляется переход *
 * на заданную метку                                                      *
 *************************************************************************/
#define UEFI_CHECK_APDU_STATUS_AND_LOG(msg, rapdu, rapdu_len, status, label)                              \
    do {                                                                                                  \
        Print(L"%s", msg);                                                                                \
        if (rapdu_len < 2) {                                                                              \
            Print(L" -> Failed with incorrect RAPDU buffer\n");                                           \
            status = EFI_PROTOCOL_ERROR;                                                                  \
            goto label;                                                                                   \
        }                                                                                                 \
        if (rapdu[rapdu_len - 2] == 0x90 && rapdu[rapdu_len - 1] == 0x00) {                               \
            Print(L" -> OK\n");                                                                           \
        } else {                                                                                          \
            Print(L" -> Failed with SW1: %02X, SW2: %02X\n", rapdu[rapdu_len - 2], rapdu[rapdu_len - 1]); \
            status = EFI_PROTOCOL_ERROR;                                                                  \
            goto label;                                                                                   \
        }                                                                                                 \
    } while (0)