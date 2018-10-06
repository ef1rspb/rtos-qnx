# rtos-qnx
Материалы по дисциплине "Программирование систем реального времени", ЛЭТИ АПУ, Дорогов А.Ю.

# Как это запустить?
1. Регистрируемся на `qnx.com`, пишем в саппорт, что являетесь студентом института ХХХ, хотите получить лицензию в образовательных целях по дисциплине "Программирование систем реального времени".
2. Через пару дней получаем ключ по почте, заходим в ЛК `myQNX`, скачиваем `QNX SDP 7.0` (на момент написания), выбираем версию под свою ОС.
3. После установки запускаем `QNX Software Center`, вкладка Available, скачиваем образ "x86-64 virtual machine for VMware", хранится в `QNX_ROOT/vmimages`.
4. Открываем образ в VMware Workstation/Fusion, запускаем `Neutrino` 
<br/>![Neutrino](/getting-started/run-neutrino.png)
5. В консоле вводим `ifconfig` ![ifconfig](/getting-started/ifconfig.png).

  Записываем ip адрес вирт. машины, пригодится.

6. В `QNX Software Center` во вкладке `Perspective (Welcome)` запускаем `Momentics (Launch Momentics)`, создаем Цель с помощью `New Launch Target` 
![New Launch Target](/getting-started/new-launch-target.png)
  <br/>Указываем ip из пункта 5, порт по умолчанию.<br/><br/>
7. `Window - Show view - Target file system navigator` ![Target file system navigator](/getting-started/show-target-fs.png).

8. Перед собой вы видем структуру файловой системы вирт. машины `Neutrino`, в какой-то из папок необходимо разместить исполняемый файл `robotNew`, я выбрал `/home/qnxuser`. Перетаскиваем файл robotNew из корня репозитория в Momentics в выбранную папку.
![Target file system](/getting-started/target-fs.png)<br/>
9. В вирт. машине переходим в папку командой `cd`, запускаем робот-эмулятор `./robotNew`. Робот отобразит свои начальные координаты.
![Run robot](/getting-started/run-robot.png)<br/>
10. В Momentics открываем один из проектов через `File - Open projects from file system`. Указываем путь до проекта, открываем, запускаем через `Run`, дебажитим через `Debug`.

# Остались вопросы?
Создайте issue или пишите на почту.
