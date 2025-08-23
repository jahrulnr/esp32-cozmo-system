#include "../setup.h"

Notification* notification;

void setupNotification() {
	if (!notification) {
		notification =
			new Notification();
	}
}