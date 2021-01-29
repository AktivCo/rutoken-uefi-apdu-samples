## Описание проекта

Этот проект содержит пример по работе с токенами и смарт-картами Рутокен из UEFI-модуля. Для взаимодействия со смарт-картами используется [UEFI Smart Card Reader Protocol](https://www.uefi.org/sites/default/files/resources/UEFI%202_5.pdf#G39.1357371), представленный в [спецификации UEFI версии 2.5](https://www.uefi.org/sites/default/files/resources/UEFI%202_5.pdf).

В примере производится обнаружение подключенной смарт-карты и обмен APDU-командами для получения серийного номера токена, логина на токен при помощи PIN-кода "12345678" и сброса прав доступа к токену.

Цель данного примера -- познакомить разработчика с принципами использования смарт-карт Рутокен из среды UEFI. К ознакомлению доступна также opensource-реализация [UEFI Smart Card Reader Protocol](https://github.com/AktivCo/uefi-smartcard-reader-driver).

## Использование UEFI Smart Card Reader Protocol

Использование UEFI Smart Card Reader Protocol осложнено тем, что в спецификации UEFI он появился только в версии 2.5, и некоторые прошивки UEFI все еще не содержат реализации этого протокола. 

Существует [opensource-реализация UEFI Smart Card Reader Protocol](https://github.com/LudovicRousseau/edk2/tree/SmartCard) в виде отдельного драйвера, выполненная [LudovicRousseau](https://github.com/LudovicRousseau/). Реализация драйвера не проходила тщательного аудита кода и [не была принята в кодовую базу проекта edk2](https://www.mail-archive.com/edk2-devel@lists.sourceforge.net/msg14937.html), в основном, из-за неготовности мейнтейнеров проекта edk2 вносить в проект компоненты, лицензированные под LGPL. 

Код драйвера с небольшими исправлениями сборки и обновленной зависимостью от EDK II размещен для ознакомления в репозитории [AktivCo/uefi-smartcard-reader-driver](https://github.com/AktivCo/uefi-smartcard-reader-driver). Эта версия драйвера может быть использована для запуска UEFI-модулей, взаимодействующих со смарт-картами, в UEFI-средах, где отсутствует встроенная реализация UEFI Smart Card Reader Protocol. После аудита кода и при условии соблюдения лицензии LGPL, в соответствии с которой распространяется драйвер, он может быть использован в UEFI-модулях, решающих прикладные задачи.

## Конфигурация и сборка

В репозитории проекта используются сабмодули. Перед сборкой, пожалуйста, убедитесь, что исходный код проекта скачан полностью. Для скачивания сабмодулей достаточно выполнить следующую команду из корневой директории проекта:

```
git submodule update --init --recursive --depth=1
```

Для сборки проекта используется окружение [EDK II](https://github.com/tianocore/edk2). Для сборки проекта должны быть выполнены [предусловия](https://github.com/tianocore/tianocore.github.io/wiki/Getting-Started-with-EDK-II), включающие в себя установку инструментов для сборки. В частности, для сборки потребуется компилятор (GCC или Clang), Python, NASM.

### Сборка в Linux (gcc)

Для настройки среды сборки, воспользуйтесь инструкциями по [ссылке](https://github.com/tianocore/tianocore.github.io/wiki/Using-EDK-II-with-Native-GCC).

Для операционных систем, основанных на Debian, ожидается, что данная команда установит все необходиные инструменты:

```
sudo apt-get install build-essential uuid-dev iasl gcc-5 nasm python3-distutils
```

Для сборки используется компилятор gcc. Тестирование сборки производилось компиляторами gcc 4.8.

Каждый шаг сборки выполняется из корневой директории проекта.

1. Выполнить сборку `BaseTools` проекта edk2:

	```
	pushd edk2
	make -C BaseTools
	popd
	```

2. Настроить `BaseTools` и переменные окружения:
	
	```
	export WORKSPACE=$(pwd)
	export PACKAGES_PATH="$(pwd)/edk2:$(pwd)"
	./edk2/edksetup.sh
	```

3. Выполнить сборку UEFI-модуля примера:

	```
	build -t GCC5 -a X64 -b RELEASE -p ./RutokenSamplesPkg/RutokenSamplesPkg.dsc
	```

Исполняемый модуль примера доступен по пути `${WORKSPACE}/Build/RutokenSamplesPkg/RELEASE_GCC5/X64/RutokenGetTokenID.efi`.

### Сборка в Windows (clang)

Перед сборкой требуется установить инструменты:
1) [Python](https://www.python.org/) версии 3.7 или выше
2) NASM - [инструкция по настройке](https://github.com/tianocore/tianocore.github.io/wiki/Nasm-Setup)
3) ASL - [инструкция по настройке](https://github.com/tianocore/tianocore.github.io/wiki/Asl-Setup)
4) [Clang](https://releases.llvm.org/download.html).

Для сборки используется компилятор clang. Тестирование сборки производилось компиляторами clang 9 и clang 11.

Каждый шаг сборки выполняется в командной строке из корневой директории проекта.

1. Настроить переменные окружения:
	```
	set NASM_BIN=C:\Path\To\Your\NASMDirectory\
	set CLANG_BIN=C:\Path\To\Your\ClangDirectory\
	set PYTHON_HOME=C:\Path\To\Your\PythonRootDirectory\
	set WORKSPACE=%CD%
	set PACKAGES_PATH=%CD%\edk2;%CD%
	```

2. Выполнить сборку `BaseTools` проекта edk2:

	```
	.\edk2\edksetup.bat Rebuild	
	```

3. Выполнить сборку UEFI-модуля примера:

	```
	build -t CLANGPDB -a X64 -b RELEASE -p ./RutokenSamplesPkg/RutokenSamplesPkg.dsc
	```

Исполняемый модуль примера доступен по пути `${WORKSPACE}/Build/RutokenSamplesPkg/RELEASE_CLANGPDB/X64/RutokenGetTokenID.efi`.

## Запуск

Для модулей, решающих прикладные задачи, стандартным методом внедрения в процесс загрузки компьютера является модификация глобальных NVRAM-переменных (см. [3 Boot Manager Unified Extensible Firmware Interface Specification 2.6](https://www.uefi.org/sites/default/files/resources/UEFI%20Spec%202_6.pdf#G7.1001195)). Для запуска UEFI-приложения, в частности, требуется создание переменной Boot#### и внесение имени этой переменной в список загрузки, заданный в переменной BootOrder. Для загрузки драйвера, аналогично, требуется создание переменной Driver#### и внесение имени этой переменной в список загрузки, заданного в переменной DriverOrder.

Поскольку пример предполагается использовать в целях ознакомления, его запуск предлагается выполнять вручную из UEFI Shell. Ниже описаны также и альтернативные методы запуска.

### Запуск с помощью UEFI Shell
1) Подготовить загрузочный раздел UEFI Shell (например, используя USB носитель). Для этого требуется:
   1) Предварительно отформатировать загрузочный раздел, использовав `FAT32` в качестве целевого формата файловой системы
   2) Создать в корне раздела директории, имеющие следующую структуру:
	```
	/EFI/boot
	```
   3) Загрузить [UEFI Shell](https://github.com/tianocore/edk2/blob/UDK2018/ShellBinPkg/UefiShell/X64/Shell.efi), переименовать
      его в `bootx64.efi` и поместить в директорию `/EFI/boot`
   2) Загрузить исполняемые образы Smart Card Reader драйвера и примеров в произвольную директорию внутри раздела
2) Выполнить загрузку с UEFI Shell раздела:
   1) Если активирована функция `Secure Boot`, её требуется отключить в настройках UEFI
   2) После отключения `Secure Boot`, возможно выполнить загрузку в UEFI Shell, выбрав содержащий UEFI Shell раздел
      в списке загрузочных разделов, предлагаемых настройками UEFI
3) Перейти в раздел, который содержит файлы исполняемых образов. Например, если файлы содержатся в разделе fs0,
   перейти в него можно командой 
	```
	fs0:
	```
4) Загрузить Smart Card Reader драйвер с помощью команды:
	```
	load SmartCardReader.efi
	```
	При удачной загрузке драйвер будет присутствовать в выводе команды `drivers`.
5) Запустить пример:
	```
	RutokenGetTokenID.efi
	```
### Запуск с помощью OVMF в системе Linux
Находясь в корневой директории выполните следующие действия:
1) Для настройки `BaseTools` и переменных окружения выполните команды:
	```
	export WORKSPACE=`pwd`
	export PACKAGES_PATH="$(pwd)/edk2:$(pwd)"
	./edk2/edksetup.sh
	```
2) Для сборки образа OVMF выполните следующую команду:
	```
	build -t GCC5 -a X64 -b RELEASE -p ./OvmfPkg/OvmfPkgX64.dsc
	```
	Данная последовательность команд соберет образ OVMF компилятором GCC версии >=5.0 (список поддерживаемых 
	идентификаторов находится в файле `edk2/Conf/tools_def.txt`) для архитектуры `x86-64`. Собранный образ OVMF 
	будут находиться по пути `Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd`.
3) Создайте рабочую директорию и директорию, в которую поместятся собранные файлы драйвера и примеров:
	```
	mkdir run-ovmf
	cd run-ovmf
	mkdir hda-contents
	```
4) Скопируйте файлы `OVMF_CODE.fd` и `OVMF_VARS.fd` из директории собранного OVMF образа в директорию `run-ovmf`:
	```
	cp ../Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd ./
	cp ../Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd ./
	```
5) Поместите файлы драйвера и примеров в директорию `hda-contents`
6) Запустите OVMF образ в `qemu`, предварительно указав путь до директории, содержащей собранные драйвер и примеры, например:
	```
	sudo qemu-system-x86_64 -L . --hda fat:rw:hda-contents \
 	-drive if=pflash,format=raw,unit=0,readonly=on,file=OVMF_CODE.fd \
	-drive if=pflash,format=raw,unit=1,readonly=off,file=OVMF_VARS.fd \
	-net none -usb -device qemu-xhci,id=xhci \
	-device usb-host,bus=xhci.0,vendorid=0x0a89,productid=0x0030
	```
	Данная команда запустит OVMF образ, предоставив виртуальной машине доступ на чтение и запись к директории `hda-contents`.
	При запуске виртуальной машины будет создано устройство виртуального USB контроллера, к которому будет подключено
	прокинутое с хоста USB устройство, указанное в параметрах `vendorid` и `productid`.
7) Перейдите в раздел, который содержит файлы исполняемых образов. Например, если файлы содержатся в разделе fs0, перейти в него можно командой
	```
	fs0:
	```
8) Загрузите Smart Card Reader драйвер с помощью команды
	```
	load SmartCardReader.efi
	```
	При удачной загрузке драйвер будет присутствовать в выводе команды `drivers`.
8) Запустите пример:
	```
	RutokenGetTokenID.efi
	```

### Установка примера в качестве загрузчика в OVMF

Для настройки автоматической загрузки драйвера и автоматического выполнения примера как основного загрузчика необходимо добавить информацию об этих модулях в UEFI-переменные Boot####, Driver####, BootOrder, DriverOrder. Выполнить это можно из среды UEFI Shell, которая загружается при выполнении шагов 1 -- 7 предыдущей инструкции, после чего выполнить:
1) Настроить автоматическую загрузку драйвера:
    ```
    bcfg driver add 0 fs0:SmartCardReader.efi "SmartCard Reader Driver"
    ```
2) Настроить автоматический запуск примера, как первого исполняемого загрузчика:
    ```
    bcfg boot add 0 fs0:RutokenGetTokenID.efi "Rutoken Sample Bootloader"
    ```
3) Завершить работу qemu.

Записи, ассоциированные с драйвером SmartCardReader.efi и загрузчиком RutokenGetTokenID.efi были записаны в NVRAM-переменные, которые сохраняются между запусками qemu в файле `OVMF_VARS.fd`. Для повторного запуска, при котором произойдет автоматическая загрузка драйвера и автоматическое выполнение примера, можно использовать команду:
```
sudo qemu-system-x86_64 -L . --hda fat:rw:hda-contents \
-drive if=pflash,format=raw,unit=0,readonly=on,file=OVMF_CODE.fd \
-drive if=pflash,format=raw,unit=1,readonly=off,file=OVMF_VARS.fd \
-net none -usb -device qemu-xhci,id=xhci \
-device usb-host,bus=xhci.0,vendorid=0x0a89,productid=0x0030
```
