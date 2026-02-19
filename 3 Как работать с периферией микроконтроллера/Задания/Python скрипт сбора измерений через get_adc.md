---
тема: Измерительный прибор
автор: Егор Анатольевич Денисов
дата: 2026-02-17
---

# Результат

- [ ] Создан Python скрипт `get_adc.py`
- [ ] Скрипт подключается к COM-порту прибора
- [ ] Скрипт в течение заданного регулярно вызывает команду `get_adc` у прибора и собирает полученные измерения
- [ ] После окончания сбора измерений скрипт строит график измерений
- [ ] Скрипт `get_adc.py` закоммичен в ваш репозиторий GitHub

# Инструкция

1. В терминале проверить, что вас установлен Python:

``` bash
python --version
```

2. Установить python модули для работы с последовательным портом и построения графиков:

``` bash
python -m pip install pyserial matplotlib
```

3. Создать в папке проекта файл python скрипта `get_adc.py`

4. В начале скрипта импортировать модули времени и последовательного порта:

``` python
import time
import serial
import matplotlib.pyplot as plt
```

5. Создать функцию чтения float из последовательного порта:

``` python
def read_value(ser):
	while True:
		try:
			line = ser.readline().decode('ascii')
			value = float(line)
			return value
		except ValueError:
			continue
```

6. Создать функцию `main`:

``` python
def main():
```

6. Далее, внутри `main` создаем объект `Serial` для работы с последовательным портом компьютера. Укажите ваше имя порта. Таймаут обязательно задать 0:

``` python
ser = serial.Serial(port='COM3', baudrate=115200, timeout=0.0)
```

7. Добавить проверку, открылся ли порт:

``` python
if ser.is_open:
	print(f"Port {ser.name} opened")
else:
	print(f"Port {ser.name} closed")
```

8. Создать листы для хранения результатов измерений и меток времени:

``` python
measure_temperature_C = []
measure_voltage_V = []
measure_ts = []
```

9. Зафиксировать время старта измерений:

``` python
start_ts = time.time()
```

10. Создать блок `try` - `finally` для гарантированного закрытия последовательного порта:

``` python
try:
	# ваш код
finally:
	ser.close()
	print("Port closed")
```

11. В теле `try` создать бесконечный цикл

12. В теле бесконечного цикла зафиксировать время измерения:

``` python
ts = time.time() - start_ts
```

13.  Отправить команду `get_adc` в последовательный порт:

``` python
ser.write("get_adc\n".encode('ascii'))
```

13. Считать ответ на команду `get_adc` c помощью написанной ранее функции `read_value`:

``` python
voltage_V = read_value(ser)
```

14. Отправить команду `get_temp` в последовательный порт
15. Считать ответ на команду `get_temp` c помощью написанной ранее функции `read_value`
16. Добавить метку времени, напряжение и температуру в листы с результатами измерений:

``` python
measure_ts.append(ts)
measure_voltage_V.append(voltage_V)
measure_temperature_C.append(temp_C)
```

17. Вывести отформатированные измерения:

``` python
print(f'{voltage_V:.3f} V - {temp_C:.1f}C - {ts:.2f}s')
```

18. Добавить задержку для ограничения скорости работы бесконечного цикла:

``` python
time.sleep(0.1)
```

19. В конце файла добавить вызов функции `main`:
``` python
if __name__ == "__main__":
	main()
```
20. запустить скрипт с прошивкой `03-adc`, убедиться, что данные считываются и выводятся в терминал:

![[Pasted image 20260219014727.png]]

21. В блок `finally` добавить построение графика зависимости напряжения и температуры от времени:

``` python
plt.subplot(2, 1, 1)
plt.plot(measure_ts, measure_voltage_V)
plt.title('График зависимости напряжения от времени')
plt.xlabel('время, с')
plt.ylabel('напряжение, В')

plt.subplot(2, 1, 2)
plt.plot(measure_ts, measure_temperature_C)
plt.title('График зависимости температуры от времени')
plt.xlabel('время, с')
plt.ylabel('температура, C')

plt.tight_layout()
plt.show()
```

22. Запустить скрипт, подождать, прервать его командой `Ctrl+C`, после чего должен быть получен график:

![[Pasted image 20260219015510.png]]

23. Сделать `commit` и `push` скрипта `get_adc.py`
