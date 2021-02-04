/**************************************************************************
 * Rutoken                                                                *
 * Copyright (c) 2020-2021, Aktiv-Soft JSC. All rights reserved.          *
 * Подробная информация:  http://www.rutoken.ru                           *
 *------------------------------------------------------------------------*
 * Данный файл содержит реализацию вспомогательных функций                *
 *************************************************************************/

/*************************************************************************
 * Включение файлов:                                                     *
 *  - utils.h - для доступа к объявлениям реализуемых в файле функций    *
 *  - Library/BaseMemoryLib.h - для доступа к функциям CopyMem и ZeroMem *
 *  - Library/UefiBootServicesTableLib.h - для доступа к указателям на   *
 *    EFI System Table и EFI Boot Services Table                         *
 *  - Library/UefiLib.h - для доступа к функции Print                    *
 ************************************************************************/
#include "utils.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

void Pause(IN CONST CHAR16 *Msg) {
    UINTN KeyEvent = 0;

    Print(L"%s\n", Msg);

    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &KeyEvent);
    gST->ConIn->Reset(gST->ConIn, FALSE);
}
