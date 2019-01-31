#pragma once

char configSSID[WIFI_SSID_LENGTH];

std::unique_ptr<DNSServer> dnsServer;
const byte DNS_PORT = 53;


int startWifiConfig(struct process * wifiConfigProcess)
{
	WiFi.mode(WIFI_AP);

	delay(100);

	WiFi.softAP(settings.deviceName);

	dnsServer.reset(new DNSServer());

	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);

	delay(500);

	return PROCESS_OK;
}

int updateWifiConfig(struct process * wifiConfigProcess)
{
	return PROCESS_OK;
}

int stopWifiConfig(struct process * wifiConfigProcess)
{
	return PROCESS_OK;
}

void wifiConfigStatusMessage(struct process * wifiConfigProcess, char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "Soft AP running");
}
