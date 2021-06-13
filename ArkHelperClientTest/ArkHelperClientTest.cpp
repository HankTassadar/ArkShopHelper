#include<ArkHelperServerAndClient.h>
extern "C"
{
#include<openssl/applink.c>
};

int main()
{
	auto instance = ArkHelper::ArkHelperServerAndClient::GetInstance(16338, "127.0.0.1");
	instance->waitForConnected();
	auto res = instance->login("76561198167294765", "96166asd");
	
}


