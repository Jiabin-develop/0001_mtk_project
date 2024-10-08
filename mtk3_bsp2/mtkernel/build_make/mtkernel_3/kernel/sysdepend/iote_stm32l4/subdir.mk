################################################################################
# micro T-Kernel 3.00.03  makefile
################################################################################

OBJS += \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/cpu_clock.o \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/devinit.o \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/hw_setting.o \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/power_save.o 

C_DEPS += \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/cpu_clock.d \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/devinit.d \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/hw_setting.d \
./mtkernel_3/kernel/sysdepend/iote_stm32l4/power_save.d 


mtkernel_3/kernel/sysdepend/iote_stm32l4/%.o: ../kernel/sysdepend/iote_stm32l4/%.c
	@echo 'Building file: $<'
	$(GCC) $(CFLAGS) -D$(TARGET) $(INCPATH) -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
