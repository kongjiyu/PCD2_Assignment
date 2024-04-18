
# Prerequisites
- Windows 7 or newer


# Windows
## 1. Download Git
Download git by this [link](https://git-scm.com/download/win)

## 2. Download Visual Studio
Download Latest Visual Studio by this [link](https://visualstudio.microsoft.com/zh-hans/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022&source=VSLandingPage&cid=2030&passive=false)

## 3. Download vcpkg
- open Command Prompt
- Go to ```C:\src``` to clone vcpkg
```cmd
cd \
mkdir src
cd src
git clone https://github.com/microsoft/vcpkg.git
```

- download vcpkg
```cmd
.\bootstrap-vcpkg.bat
```

- Search pdcurses library and make sure there is pdcurses library
```cmd
.\vcpkg search pdcurses
```

- Download pdcurses library
```cmd
.\vcpkg install pdcurses
```

- Integrate to link to visual studio code
```cmd
.\vcpkg integrate install
