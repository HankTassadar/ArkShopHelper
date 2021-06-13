#include <iostream>
#include<ArkHelperServerAndClient.h>

extern "C"
{
#include<openssl/applink.c>
};

int main()
{
    ArkHelper::ArkHelperServerAndClient::GetInstance();
}
