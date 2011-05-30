void wsdisplay_write_palette(int, int, int, int);
void wsmouse_io(void);
void wskbd_io(void);

void wscons_bell(int, DeviceIntPtr, pointer, int);

int wskbd_init(void);
int wsmouse_init(void);
int wsdisplay_init(ScreenPtr, int, char **);

void wsdisplay_shutdown(void);

void wsdisplay_closedown(void);
void wskbd_closedown(void);

int mouse_accel(DeviceIntPtr, int);
