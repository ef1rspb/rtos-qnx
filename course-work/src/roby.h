#ifndef ROBY_H_
#define ROBY_H_

// Биты регистра A:
#define  A_D            0x01 // D: управление дрелью;
#define  A_S	        0x02 // S: управление схватом;
#define  A_X_FORWARD 	0x04 // +X: движение по X вперёд;
#define  A_X_BACK		0x08 // -X: движение по X назад;
#define  A_Z_BACK    	0x10 // -Z: движение по Z назад;
#define  A_Z_FORWARD 	0x20 // +Z: движение по Z вперёд;
#define  A_Y_BACK    	0x40 // -Y: движение по Y назад;
#define  A_Y_FORWARD	0x80 // +Y: движение по Y вперёд.

// Биты регистра B:
#define  B_X            0x01 // Xp (Xpulse): импульсный датчик движения по X;
#define  B_Y            0x02 // Yp (Ypulse): импульсный датчик движения по Y;
#define  B_Z            0x04 // Zp (Zpulse): импульсный датчик движения по Z;
#define  B_W_END		0x08 // WE (WEnd): датчик конечного положения по W (для головки);
#define  B_W_BEGIN		0x10 // WB (WBegin): датчик начального положения по W (для головки);
#define  B_Z_BEGIN		0x20 // ZN: датчик начального положения по Z;
#define  B_Y_BEGIN		0x40 // YN: датчик начального положения по Y;
#define  B_X_BEGIN		0x80 // XN: датчик начального положения по X.

// Биты регистра C: (при этом биты PC0 и PC1 не используются)
#define  C_F_END        0x04 // FE (FEnd): датчик конечного положения по F (для основания);
#define  C_F_BEGIN      0x08 // FB (FBegin): датчик начального положения по F (для основания);
#define  C_W_FORWARD 	0x10 // +W: движение по W вперёд (для головки);
#define  C_W_BACK    	0x20 // -W: движение по W назад (для головки);
#define  C_F_FORWARD 	0x40 // +F: движение по F вперёд (для основания);
#define  C_F_BACK    	0x80 // -F: движение по F назад (для основания).

// Для потока DisplayXYZ
//Коды сигналов, передающих значения координат X, Y и Z от робота Roby:
#define X_VALUE		-1 //Для кодов сигналов используются отрицательные значения,
#define Y_VALUE		-2 //так как положительные значения кодов сигналов
#define Z_VALUE		-4 //использует система.

// Для потока DisplayWF
// Коды (поле code) импульсных сообщений от датчиков крайних положений по координатам W и F:
#define W_END		0x08
#define W_BEGIN		0x10
#define F_END		0x20
#define F_BEGIN		0x40

// Коды импульсных сообщений для запуска таймеров в потоке DisplayWF:
#define TIMER_W_SET	1
#define TIMER_F_SET	2

// Коды импульсов таймера для заполнения структуры (уведомления) sigevent в потоке DisplayWF:
#define TIMER_W_COUNT	33 // таймер координаты W;
#define TIMER_F_COUNT	65 // таймер координаты F.

#endif /* ROBY_H_ */
