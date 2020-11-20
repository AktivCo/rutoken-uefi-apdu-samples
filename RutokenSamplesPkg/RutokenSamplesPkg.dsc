## @file
#  **************************************************************************
#  * Rutoken                                                                *
#  * Copyright (c) 2020-2021, Aktiv-Soft JSC. All rights reserved.          *
#  * Подробная информация:  http://www.rutoken.ru                           *
#  *------------------------------------------------------------------------*
#  * Пример работы с Рутокен В UEFI на языке C                              *
#  **************************************************************************
##

[Defines]
  PLATFORM_NAME                  = RutokenSamplesPkg
  PLATFORM_GUID                  = 8a1e606f-a925-438d-93eb-51580735ce15
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/RutokenSamplesPkg
  SUPPORTED_ARCHITECTURES        = IA32|IPF|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  #
  # Common Libraries
  #
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf

[Components]

#### SmartCardReader Applications.
  RutokenSamplesPkg/RutokenGetTokenID/RutokenGetTokenID.inf

