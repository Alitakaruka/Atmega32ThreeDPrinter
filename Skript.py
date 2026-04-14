import math

ADC_RES = 1024
BETA = 3950.0
if top:р 100k
T0 = 298.15    # 25 °C
R_FIXED = 100000.0  # resistor 10k
top = True

table = []

for adc in range(ADC_RES):
    if adc == 0 or adc == 1023:
        table.append(-273.15)
        continue

    if top:
        resistance = R_FIXED * ((ADC_RES / adc) - 1.0)
    else:
        resistance = R_FIXED * (adc / (ADC_RES - adc))

    temp_k = 1.0 / ((1.0 / T0) + (1.0 / BETA) * math.log(resistance / R0))
    temp_c = temp_k - 273.15
    table.append(int(temp_c))
print("const int nozzle_temp_table[1024] PROGMEM = {")
for i, temp in enumerate(table):
    print(f"  {temp:.0f},", end='')
    if i % 8 == 7:
        print()
print("\n};")
