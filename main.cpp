#include <iostream>
#include <fstream>
#include <httplib/httplib.h>
#include <base64/base64.h>
#include <windows.h>

using namespace std;

typedef int(__stdcall *pSDT_OpenPort)(int);

typedef int(__stdcall *pSDT_ClosePort)(int);

typedef int(__stdcall *pSDT_StartFindIDCard)(int, unsigned char *, int);

typedef int(__stdcall *pSDT_SelectIDCard)(int, unsigned char *, int);

typedef int(__stdcall *pSDT_ReadBaseMsg)(int, unsigned char *, unsigned int *, unsigned char *, unsigned int *,
                                         int);

typedef int(_cdecl
*pSDT_Unpack)(char *, char *, int);

HINSTANCE hGetProcIDDLL = LoadLibraryA("sdtapi.dll");
HINSTANCE hGetProcWLTDLL = LoadLibraryA("DLL_File.dll");

pSDT_OpenPort OpenPort = (pSDT_OpenPort) GetProcAddress(hGetProcIDDLL, "SDT_OpenPort");
pSDT_ClosePort ClosePort = (pSDT_ClosePort) GetProcAddress(hGetProcIDDLL, "SDT_ClosePort");
pSDT_StartFindIDCard StartFindIDCard = (pSDT_StartFindIDCard) GetProcAddress(hGetProcIDDLL, "SDT_StartFindIDCard");
pSDT_SelectIDCard SelectIDCard = (pSDT_SelectIDCard) GetProcAddress(hGetProcIDDLL, "SDT_SelectIDCard");
pSDT_ReadBaseMsg ReadBaseMsg = (pSDT_ReadBaseMsg) GetProcAddress(hGetProcIDDLL, "SDT_ReadBaseMsg");
pSDT_Unpack Unpack = (pSDT_Unpack) GetProcAddress(hGetProcWLTDLL, "unpack");

int iRet = 0; //返回码
int iPort = 1001; //端口号
int iIfOpen = 0; //是否需要打开端口
unsigned char pucManaInfo[4] = {0};
unsigned char pucManaMsg[8] = {0};
unsigned char pucCHMsg[256] = {0}; //文字信息最长 256 字节
unsigned char pucPHMsg[1024] = {0}; //相片信息最长 1024 字节
unsigned char pucFPMsg[1024] = {0}; //指纹信息最长 1024 字节
unsigned int uiCHMsgLen, uiPHMsgLen, uiFPMsgLen = 0;
unsigned char pucBgrBuffer[38556] = {0};    //解码后图片BGR编码值
unsigned char pucBmpBuffer[38862] = {0};    //解码后图片RGB编码值

int do_reader() {
    iRet = OpenPort(iPort);
    if (iRet != 144)
        return -1;
    iRet = StartFindIDCard(iPort, pucManaInfo, iIfOpen);
    if (iRet != 159)
        return -2;
    iRet = SelectIDCard(iPort, pucManaMsg, iIfOpen);
    if (iRet != 144)
        return -3;
    iRet = ReadBaseMsg(iPort, pucCHMsg, &uiCHMsgLen, pucPHMsg, &uiPHMsgLen, iIfOpen);
    if (iRet != 144)
        return -4;
    iRet = Unpack((char *) pucPHMsg, (char *) pucBgrBuffer, 1);
    if (iRet != 1)
        return -5;
    iRet = ClosePort(iPort);
    return 1;
}

std::string do_zp() {
    //打开图片
    FILE *fp = NULL;
    unsigned int size;         //图片字节数
    char *buffer;
    size_t result;

    fp = fopen("zp.bmp", "rb");
    if (NULL == fp) {
        printf("open_error");
        exit(1);
    }


    //获取图片大小
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    //分配内存存储整个图片
    buffer = (char *) malloc((size / 4 + 1) * 4);
    if (NULL == buffer) {
        printf("memory_error");
        exit(2);
    }

    //将图片拷贝到buffer
    result = fread(buffer, 1, size, fp);
    if (result != size) {
        printf("reading_error");
        exit(3);
    }
    fclose(fp);

    std::string encoded = base64_encode(reinterpret_cast<const unsigned char *>(buffer), size);
    free(buffer);
    return "data:image/bmp;base64," + encoded;
}

std::string read() {
    int iRet = 0;
    iRet = do_reader();
    if (iRet == -1)
        return "端口打开失败！";
    else if (iRet == -2)
        return "找卡失败！";
    else if (iRet == -3)
        return "选卡失败！";
    else if (iRet == -4)
        return "读卡失败！";
    else if (iRet == -5)
        return "图片失败！";
    std::string img = do_zp();
    return img;
//
//    wchar_t Name[15 + 1] = {0};//张5f20
//    wchar_t wszName[15 + 1] = {0};
//    wchar_t wszSex[1 + 1] = {0};            //性别
//    wchar_t wszNation[2 + 1] = {0};         //民族
//    wchar_t wszBirth[8 + 1] = {0};         //出生
//    wchar_t wszAddr[35 + 1] = {0};          //住址
//    wchar_t wszID[18 + 1] = {0};            //公民身份号码
//    wchar_t wszDept[15 + 1] = {0};          //签发机关
//    wchar_t wszStart[8 + 1] = {0};         //有效期起始
//    wchar_t wszEnd[8 + 1] = {0};           //有效期截至
//    //截取个人信息数据。信息采用UNICODE存储，具体格式参可见《二代证机读信息说明.doc》
//    memcpy_s(Name, sizeof(Name), &pucCHMsg[0], 30);
//    memcpy_s(wszName, sizeof(wszName), &pucCHMsg[0], 30);
//    memcpy_s(wszSex, sizeof(wszSex), &pucCHMsg[30], 2);
//    memcpy_s(wszNation, sizeof(wszNation), &pucCHMsg[32], 4);
//    memcpy_s(wszBirth, sizeof(wszBirth), &pucCHMsg[36], 16);
//    memcpy_s(wszAddr, sizeof(wszAddr), &pucCHMsg[52], 70);
//    memcpy_s(wszID, sizeof(wszID), &pucCHMsg[122], 36);
//    memcpy_s(wszDept, sizeof(wszDept), &pucCHMsg[158], 30);
//    memcpy_s(wszStart, sizeof(wszStart), &pucCHMsg[188], 16);
//    memcpy_s(wszEnd, sizeof(wszEnd), &pucCHMsg[204], 16);
//
//    _wsetlocale(LC_ALL, L"chs");
//    wprintf(L"%s\n", wszName);
//    wprintf(L"%s\n", wszSex);
//    wprintf(L"%s\n", wszNation);
//    wprintf(L"%s\n", wszBirth);
//    wprintf(L"%s\n", wszAddr);
//    wprintf(L"%s\n", wszID);
//    wprintf(L"%s\n", wszDept);
//    wprintf(L"%s\n", wszStart);
//    printf("%s\n", wszEnd);

};

int main(void) {
    using namespace httplib;
    Server svr;
    svr.Get("/", [](const Request &req, Response &res) {
        res.set_content("欢迎使用", "text/html;charset=UTF-8");
    });
    svr.Get("/read", [](const Request &req, Response &res) {
        std::string str = read();
        cout << str << endl;
        res.set_content(str, "text/html;charset=UTF-8");
    });
    svr.listen("127.0.0.1", 1234);
}

