/**************************************************************************
 * Rutoken                                                                *
 * Copyright (c) 2020-2021, Aktiv-Soft JSC. All rights reserved.          *
 * Подробная информация:  http://www.rutoken.ru                           *
 *------------------------------------------------------------------------*
 * Пример работы с Рутокен В UEFI на языке C                              *
 *------------------------------------------------------------------------*
 * Использование команд получения информации о доступных токенах:         *
 *  - нахождение всех подключенных токенов;                               *
 *  - установление соединения с Рутокен в первом доступном слоте;         *
 *  - получение ID токена;                                                *
 *  - выполнение аутентификации Пользователя;                             *
 *  - сброс прав доступа Пользователя на Рутокен и закрытие соединения    *
 *    с Рутокен.                                                          *
 *------------------------------------------------------------------------*
 * Для работы примера требуется наличие загруженного в память UEFI        *
 * драйвера, реализующего протокол Smart Card Reader Protocol.            *
 *************************************************************************/

/*************************************************************************
 * Включение файлов:                                                     *
 *  - Uefi.h - для доступа к определениям из спецификации UEFI и базовым *
 *    типам UEFI                                                         *
 *  - Library/BaseMemoryLib.h - для доступа к функциям CopyMem и ZeroMem *
 *  - Library/UefiBootServicesTableLib.h - для доступа к указателям на   *
 *    EFI System Table и EFI Boot Services Table                         *
 *  - Library/UefiLib.h - для доступа к функции Print                    *
 *  - Protocol/SmartCardReader.h - для доступа к протоколу               *
 *    Smart Card Reader Protocol                                         *
 *  - common.h - для доступа к вспомогательным данным примера            *
 *  - log.h - для доступа к инструментам логирования                     *
 *  - utils.h - для доступа к вспомогательным функциям                   *
 ************************************************************************/
#include <Library/BaseMemoryLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Uefi.h>

#include <Protocol/SmartCardReader.h>

#include "common.h"
#include "log.h"
#include "utils.h"

/**************************************************************************
 * Функция инициации подключения с картой                                 *
 *************************************************************************/
EFI_STATUS SmartCardConnect(EFI_SMART_CARD_READER_PROTOCOL* SmartCardReader, UINT32* Protocol) {
    EFI_STATUS Status = EFI_SUCCESS; // код статуса выполнения

    /**************************************************************************
     * Инициировать подключение с картой                                      *
     *************************************************************************/
    Status = SmartCardReader->SCardConnect(SmartCardReader, SCARD_AM_CARD, SCARD_CA_COLDRESET,
                                           SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, Protocol);
    UEFI_CHECK_ERROR_AND_LOG(L"  SCardConnect", Status, StatusToStr(Status), Exit);

Exit:
    return Status;
}

/**************************************************************************
 * Функция обмена данными с картой                                        *
 *************************************************************************/
EFI_STATUS SmartCardTransmit(EFI_SMART_CARD_READER_PROTOCOL* SmartCardReader, UINT32 Protocol, UINT8* CAPDU,
                             UINTN CAPDULength, UINT8* RAPDU, UINTN* RAPDULength) {
    EFI_STATUS Status = EFI_SUCCESS;     // код статуса выполнения
    UINTN RAPDUOldLength = *RAPDULength; // сохранённый размер выделенного буфера RAPDU
    UINT8 ApduCase = -1;                 // Case текущей команды

    UEFI_LOG_XXD(CAPDU, CAPDULength, L"  Command APDU: ");

    if (Protocol == SCARD_PROTOCOL_T0) {
        /**************************************************************************
         * Определить Case APDU команды                                           *
         *************************************************************************/
        if (CAPDULength == 4)
            ApduCase = 1;
        else if (CAPDULength == 5)
            ApduCase = 2;
        else if (CAPDULength > 5) {
            if (CAPDU[4] + 5 == CAPDULength)
                ApduCase = 3;
            else if (CAPDU[4] + 5 + 1 == CAPDULength)
                ApduCase = 4;
        }

        if (ApduCase == -1) return EFI_PROTOCOL_ERROR;

        /**************************************************************************
         * Преобразовать команду Case 4 в команду Case 3 для передачи при помощи  *
         * команды GET RESPONSE                                                   *
         *************************************************************************/
        if (ApduCase == 4) --CAPDULength;
    }

    /**************************************************************************
     * Отправить данные на карту и получить ответ                             *
     *************************************************************************/
    Status = SmartCardReader->SCardTransmit(SmartCardReader, CAPDU, CAPDULength, RAPDU, RAPDULength);
    UEFI_CHECK_ERROR_AND_LOG(L"  SCardTransmit", Status, StatusToStr(Status), Exit);

    /**************************************************************************
     * Завершить выполнение, если в ответ не был получен, как минимум, код    *
     * возврата SW1SW2                                                        *
     *************************************************************************/
    if (*RAPDULength < 2) return EFI_PROTOCOL_ERROR;

    /**************************************************************************
     * Проверка на получение в ответ статуса SW1=0x61 (ожидание приема        *
     * дополнительных байт ответа) или SW1=0x6c (неверное значение поля Le)   *
     *************************************************************************/
    if (Protocol == SCARD_PROTOCOL_T0 && *RAPDULength == 2 && (RAPDU[0] == 0x61 || RAPDU[0] == 0x6c)) {
        UINT8 LocalCAPDU[260];
        UINT8 LocalRAPDU[258];
        UINTN OutLength;
        UINTN* OutLengthPtr;
        UINT8* OutBuffer;

        OutLength = RAPDU[1] + 2;
        OutLengthPtr = &OutLength;
        OutBuffer = LocalRAPDU;

        if (ApduCase == 2 || ApduCase == 4) {
            if (RAPDU[1] + 2 > RAPDUOldLength) return EFI_BUFFER_TOO_SMALL;
            *RAPDULength = RAPDUOldLength;
            OutLengthPtr = RAPDULength;
            OutBuffer = RAPDU;
        }

        /**************************************************************************
         * Установить GET RESPONSE в качестве отправляемой команды в случае, если *
         * имеются дополнительные байты ответа                                    *
         *************************************************************************/
        if (RAPDU[0] == 0x61) {
            UINT8 GetResponseAPDU[5] = { 0x00, 0xC0, 0x00, 0x00, 0x00 };

            CopyMem(LocalCAPDU, GetResponseAPDU, sizeof(GetResponseAPDU));
            CAPDULength = sizeof(GetResponseAPDU);
        }
        /**************************************************************************
         * Использовать исходную команду в случае неправильного значения поля Le  *
         *************************************************************************/
        else if (RAPDU[0] == 0x6C) {
            CopyMem(LocalCAPDU, CAPDU, CAPDULength);
        }

        /**************************************************************************
         * Установить правильное значение для поля P3                             *
         *************************************************************************/
        LocalCAPDU[CAPDULength - 1] = RAPDU[1];

        /**************************************************************************
         * Отправить данные на карту и получить ответ                             *
         *************************************************************************/
        Status = SmartCardReader->SCardTransmit(SmartCardReader, LocalCAPDU, CAPDULength, OutBuffer, OutLengthPtr);
        UEFI_CHECK_ERROR_AND_LOG(L"  SCardTransmit", Status, StatusToStr(Status), Exit);

        /**************************************************************************
         * Завершить выполнение, если в ответ не был получен, как минимум, код    *
         * возврата SW1SW2                                                        *
         *************************************************************************/
        if (OutLength < 2) return EFI_PROTOCOL_ERROR;

        if (ApduCase != 2 && ApduCase != 4) {
            RAPDU[0] = OutBuffer[OutLength - 2];
            RAPDU[1] = OutBuffer[OutLength - 1];
        }
    }

    UEFI_LOG_XXD(RAPDU, *RAPDULength, L" Response APDU: ");

Exit:
    return Status;
}

/**************************************************************************
 * Функция прекращения соединения с картой                                *
 *************************************************************************/
EFI_STATUS SmartCardDisconnect(EFI_SMART_CARD_READER_PROTOCOL* SmartCardReader) {
    EFI_STATUS Status = EFI_SUCCESS; // код статуса выполнения

    /**************************************************************************
     * Разорвать соединение с картой                                          *
     *************************************************************************/
    Status = SmartCardReader->SCardDisconnect(SmartCardReader, SCARD_CA_UNPOWER);
    UEFI_CHECK_ERROR_AND_LOG(L"  SCardDisconnect", Status, StatusToStr(Status), Exit);

Exit:
    return Status;
}

/**************************************************************************
 * Функция аутентификации пользователя на токене                          *
 *************************************************************************/
EFI_STATUS SmartCardLogin(EFI_SMART_CARD_READER_PROTOCOL* SmartCardReader, UINT32 Protocol, CHAR8* Pin, UINTN PinLength,
                          UINTN User) {
    EFI_STATUS Status = EFI_SUCCESS;                         // код статуса выполнения
    UINT8 Apdu[260] = { 0x00, 0x20, 0x00, User, PinLength }; // APDU команда VERIFY,
                                                             // выполняющая аутентификацию
                                                             // пользователя с
                                                             // соответствующим CHV-RSF ID
    UINTN ApduLength = 5;                                    // размер APDU команды в байтах
    UINT8 RAPDU[2];                                          // массив полученных с карты байт
    UINTN RAPDULength = sizeof(RAPDU); // размер полученных с карты данных в байтах

    /**************************************************************************
     * Закончить выполнение, если длина pin кода больше предельно допустимой  *
     * длины данных APDU фрейма                                               *
     *************************************************************************/
    if (sizeof(Apdu) - ApduLength < PinLength) {
        Status = EFI_BUFFER_TOO_SMALL;
        goto Exit;
    }

    /**************************************************************************
     * Скопировать байты pin кода в APDU фрейм                                *
     *************************************************************************/
    for (int i = 0; i < PinLength; ++i) { Apdu[ApduLength++] = Pin[i]; }

    /**************************************************************************
     * Отправить байты APDU команды на карту и получить код возврата SW1-SW2  *
     *************************************************************************/
    Status = SmartCardTransmit(SmartCardReader, Protocol, Apdu, ApduLength, RAPDU, &RAPDULength);
    UEFI_CHECK_ERROR_AND_LOG(L"  SmartCardTransmit", Status, StatusToStr(Status), Exit);

    UEFI_CHECK_APDU_STATUS_AND_LOG(L" APDU VERIFY", RAPDU, RAPDULength, Status, Exit);

Exit:
    return Status;
}

/**************************************************************************
 * Функция сброса прав доступа на токене                                  *
 *************************************************************************/
EFI_STATUS SmartCardLogout(EFI_SMART_CARD_READER_PROTOCOL* SmartCardReader, UINT32 Protocol) {
    EFI_STATUS Status = EFI_SUCCESS; // код статуса выполнения
    UINT8 Apdu[] = { 0x80, 0x40, 0x00, 0x00 }; // APDU команда RESET ACCESS RIGHTS, сбрасывающая все права доступа
    UINT8 RAPDU[2];                    // массив полученных с карты байт
    UINTN RAPDULength = sizeof(RAPDU); // размер полученных с карты данных в байтах

    /**************************************************************************
     * Отправить байты APDU команды на карту и получить код возврата SW1-SW2  *
     *************************************************************************/
    Status = SmartCardTransmit(SmartCardReader, Protocol, Apdu, sizeof(Apdu), RAPDU, &RAPDULength);
    UEFI_CHECK_ERROR_AND_LOG(L"  SmartCardTransmit", Status, StatusToStr(Status), Exit);

    UEFI_CHECK_APDU_STATUS_AND_LOG(L" APDU RESET ACCESS RIGHTS", RAPDU, RAPDULength, Status, Exit);

Exit:
    return Status;
}


/**************************************************************************
 * Функция получения ID токена                                            *
 *************************************************************************/
EFI_STATUS SmartCardGetTokenId(EFI_SMART_CARD_READER_PROTOCOL* SmartCardReader, UINT32 Protocol, UINT8* TokenBuffer,
                               UINTN* TokenBufferSize) {
    EFI_STATUS Status = EFI_SUCCESS;                             // код статуса выполнения
    UINT8 Apdu[] = { 0x00, 0xCA, 0x01, 0x81, *TokenBufferSize }; // APDU команда GET DATA, получающая ID токена
    UINTN ApduLength = sizeof(Apdu);                             // размер APDU команды в байтах
    UINT8 RAPDU[TOKEN_ID_SIZE + 2];    // массив полученных с карты байт
    UINTN RAPDULength = sizeof(RAPDU); // размер полученных с карты данных в байтах

    /**************************************************************************
     * Закончить выполнение, если размер выделенного под данные буфера       *
     * меньше ожидаемого размера буфера ID токена                            *
     *************************************************************************/
    if (*TokenBufferSize < TOKEN_ID_SIZE) {
        Status = EFI_BUFFER_TOO_SMALL;
        goto Exit;
    }

    /**************************************************************************
     * Отправить байты APDU команды на карту и получить ID токена             *
     *************************************************************************/
    Status = SmartCardTransmit(SmartCardReader, Protocol, Apdu, ApduLength, RAPDU, &RAPDULength);
    UEFI_CHECK_ERROR_AND_LOG(L"  SmartCardTransmit", Status, StatusToStr(Status), Exit);

    UEFI_CHECK_APDU_STATUS_AND_LOG(L" APDU GET DATA", RAPDU, RAPDULength, Status, Exit);

    CopyMem(TokenBuffer, RAPDU, TOKEN_ID_SIZE);

Exit:
    return Status;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS Status; // код статуса выполнения
    UINTN HandleCount; // количество найденных хендлов устройств смарт карт
    EFI_HANDLE* DevicePathHandleBuffer = NULL; // массив хендлов смарт карт
    EFI_SMART_CARD_READER_PROTOCOL* SmartCardReader = NULL; // указатель на структуру, предоставляющую интерфейс
                                                            // Smart Card Reader Protocol
    UINT8 TokenId[TOKEN_ID_SIZE];        // массив байт для хранения ID токена
    UINTN TokenIdSize = sizeof(TokenId); // размер массива ID токена в байтах
    UINT32 Protocol;                     // используемый протокол (T0 или T1)

    INTN Error = 1; // флаг ошибки

    /**************************************************************************
     * Выполнить действия для начала работы с токеном                         *
     *************************************************************************/
    Print(L"Initialization...\n");

    /**************************************************************************
     * Получить хендлы всех устройств, предоставляющих протокол               *
     * EFI_SMART_CARD_READER_PROTOCOL_GUID (всех подключенных ридеров)        *
     *************************************************************************/
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSmartCardReaderProtocolGuid, NULL, &HandleCount,
                                     &DevicePathHandleBuffer);
    UEFI_CHECK_ERROR_AND_LOG(L" LocateHandleBuffer", Status, StatusToStr(Status), Exit);

    Print(L" Found %d reader(s)\n", HandleCount);

    if (HandleCount == 0) { goto FreePool; }

    /**************************************************************************
     * Получить указатель на структуру, реализующую протокол                  *
     * EFI_SMART_CARD_READER_PROTOCOL_GUID для первого найденного ридера      *
     *************************************************************************/
    Status = gBS->OpenProtocol(DevicePathHandleBuffer[0], &gEfiSmartCardReaderProtocolGuid, (VOID**)&SmartCardReader,
                               gImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    UEFI_CHECK_ERROR_AND_LOG(L" OpenProtocol", Status, StatusToStr(Status), FreePool);

    /**************************************************************************
     * Инициировать подключение к карте                                       *
     *************************************************************************/
    Status = SmartCardConnect(SmartCardReader, &Protocol);
    UEFI_CHECK_ERROR_AND_LOG(L" SmartCardConnect", Status, StatusToStr(Status), CloseProtocol);

    Print(L"Initialization has been completed successfully.\n");

    if (Protocol == SCARD_PROTOCOL_T0)
        Print(L"Using protocol T0.\n");
    else
        Print(L"Using protocol T1.\n");

    Print(L"\nReceiving token ID...\n");

    /**************************************************************************
     * Получить ID токена                                                     *
     *************************************************************************/
    Status = SmartCardGetTokenId(SmartCardReader, Protocol, TokenId, &TokenIdSize);
    UEFI_CHECK_ERROR_AND_LOG(L" SmartCardGetTokenID", Status, StatusToStr(Status), DisconnectCard);

    UEFI_LOG_XXD(TokenId, TokenIdSize, L"Token ID: ");
    Print(L"Token ID has been received successfully.\n");

    Print(L"\nLogging in...\n");

    /**************************************************************************
     * Выполнить аутентификацию пользователя                                  *
     *************************************************************************/
    Status = SmartCardLogin(SmartCardReader, Protocol, USER_PIN, USER_PIN_LEN, CKU_USER);
    UEFI_CHECK_ERROR_AND_LOG(L" SmartCardLogin", Status, StatusToStr(Status), DisconnectCard);

    Print(L"\nLogin has been completed successfully.\n");

    Print(L"\nLogging out...\n");

    /**************************************************************************
     * Сбросить права доступа                                                 *
     *************************************************************************/
    Status = SmartCardLogout(SmartCardReader, Protocol);
    UEFI_CHECK_ERROR_AND_LOG(L" SmartCardLogout", Status, StatusToStr(Status), DisconnectCard);

    Print(L"\nLogout has been completed successfully.\n");

    /**************************************************************************
     * Выставить признак успешного завершения программы                       *
     *************************************************************************/
    Error = 0;

    Print(L"\nFinalizing...\n");

DisconnectCard:

    /**************************************************************************
     * Закончить соединение с картой                                          *
     *************************************************************************/
    Status = SmartCardDisconnect(SmartCardReader);
    UEFI_CHECK_RELEASE_AND_LOG(L" SmartCardDisconnect", Status, StatusToStr(Status), Error);

CloseProtocol:

    /**************************************************************************
     * Закончить работу со SmartCardReader протоколом                         *
     *************************************************************************/
    Status = gBS->CloseProtocol(DevicePathHandleBuffer[0], &gEfiSmartCardReaderProtocolGuid, gImageHandle, NULL);
    UEFI_CHECK_RELEASE_AND_LOG(L" CloseProtocol", Status, StatusToStr(Status), Error);

FreePool:

    /**************************************************************************
     * Освободить выделенный под хендлы устройств буфер                      *
     *************************************************************************/
    Status = gBS->FreePool(DevicePathHandleBuffer);
    UEFI_CHECK_RELEASE_AND_LOG(L" FreePool", Status, StatusToStr(Status), Error);

Exit:
    if (Error) {
        Print(L"\n\nSome error occurred. Sample failed.\n");
    } else {
        Print(L"\n\nSample finished successfully.\n");
    }

    Pause(L"Press any key to continue");

    return Error;
}
