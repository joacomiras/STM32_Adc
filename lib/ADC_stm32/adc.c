#include "adc.h"
#include "stm32f103xb.h"


void adc_init() {
    // MASK CLK_ ->.      ADC1               GPIOA                  GPIOB
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

    // 2. ADON -> ANALOG TO DIGITAL ON
    ADC1->CR2 |= ADC_CR2_ADON;

    // Pequeña espera para que el ADC se estabilice (?)
    for (volatile int i = 0; i < 1000; i++);

    // 3. CAL -> CALIBRATION
    ADC1->CR2 |= ADC_CR2_CAL;
    // CAL se pone en 0 cuando termina de calibrar, sale del loop del while
    while (ADC1->CR2 & ADC_CR2_CAL);
}

float adc_read(int canal, int vMax) {
    // Configurar el pin correspondiente al canal como entrada analógica.
    // Esto es necesario hacerlo cada vez si se cambian de canales.
    // ! Agregar que si es el mismo canal no volver a configurarlo. !
    if (canal <= 7) { // Canales 0-7 están en el Puerto A (PA0-PA7)
        // Limpiar los 4 bits de configuración del pin correspondiente
        GPIOA->CRL &= ~(0x4 << (canal * 4));
    } else if (canal <= 9) { // Canales 8-9 están en el Puerto B (PB0-PB1)
        // Limpiar los 4 bits de configuración del pin correspondiente
        GPIOB->CRL &= ~(0x4 << ((canal % 8) * 4));
    }

    //SQR3-> REGULAR SEQUENCE REGISTER ; ES DONDE SE ESCRIBE EL CANAL QUE QUIERO LEER
    ADC1->SQR3 = 0; //LIMPIO
    //SMPR2 -> SAMPLE RATE ; SE USA EL 2 PORQUE VAN DE LOS CH 0 AL 9 Y EL MCU TIENE 10 CH
    ADC1->SMPR2 = 0; //LIMPIO

    // SE ESTABLECE LA PRIMERA POSICIÓN EN LA SECUENCIA PARA LEER
    ADC1->SQR3 |= (canal & 0x1F); // ESCRIBE EN SQ1

    // SE PONE EN 239.5 CICLOS QUE ES EL MAX
    // OFFSET DE 3 BITS PORQUE CADA CANAL OCUPA 3 BITS
    ADC1->SMPR2 |= (0b111 << (canal * 3));

    // 3. Iniciar la conversión (SWSTART = 1)
    ADC1->CR2 |= ADC_CR2_SWSTART;

    // SR -> STATUS REGISTER
    // EOC -> END OF CONVERTION ; CUANDO TERMINA LA CONVERSION SE PONE EN 1
    while (!(ADC1->SR & ADC_SR_EOC));

    // DR -> DATA REGISTER ; TIENE LOS DATOS DE LA CONVERSION ; AL LEER SE LIMPIA EOC
    // DR VA A DEVOLVER UN VALOR CORRESPONDIENTE A LOS BITS DEL ADC (DE 0 A 4095)
    // ENTONCES LO CONVIERTO PARA QUE ME LOS DE EN VOLTS
    return (ADC1->DR/(vMax*4095));
}