#include "setup/setup.h"

FTPServer ftpSrv(LittleFS);

void setupFTPServer() {
	ftpSrv.begin(FTP_USER, FTP_PASS);
}