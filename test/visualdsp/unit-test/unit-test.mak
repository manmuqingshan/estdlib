# Generated by the VisualDSP++ IDDE

# Note:  Any changes made to this Makefile will be lost the next time the
# matching project file is loaded into the IDDE.  If you wish to preserve
# changes, rename this file and run it externally to the IDDE.

# The syntax of this Makefile is such that GNU Make v3.77 or higher is
# required.

# The current working directory should be the directory in which this
# Makefile resides.

# Supported targets:
#     unit-test_Debug
#     unit-test_Debug_clean

# Define this variable if you wish to run this Makefile on a host
# other than the host that created it and VisualDSP++ may be installed
# in a different directory.

ADI_DSP=C:\Program Files (x86)\Analog Devices\VisualDSP 5.1.2


# $VDSP is a gmake-friendly version of ADI_DIR

empty:=
space:= $(empty) $(empty)
VDSP_INTERMEDIATE=$(subst \,/,$(ADI_DSP))
VDSP=$(subst $(space),\$(space),$(VDSP_INTERMEDIATE))

RM=cmd /C del /F /Q

#
# Begin "unit-test_Debug" configuration
#

ifeq ($(MAKECMDGOALS),unit-test_Debug)

unit-test_Debug : ./Debug/unit-test.dxe 

./Debug/ExceptionHandler-BF537.doj :./ExceptionHandler-BF537.asm ./VDK.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h 
	@echo ".\ExceptionHandler-BF537.asm"
	$(VDSP)/easmblkfn.exe .\ExceptionHandler-BF537.asm -proc ADSP-BF537 -file-attr ProjectName=unit-test -g -D_ADI_THREADS -o .\Debug\ExceptionHandler-BF537.doj -MM

Debug/MainThread.doj :MainThread.cpp MainThread.h VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h $(VDSP)/Blackfin/include/stddef.h $(VDSP)/Blackfin/include/assert.h $(VDSP)/Blackfin/include/sys/platform.h $(VDSP)/Blackfin/include/sys/_adi_platform.h $(VDSP)/Blackfin/include/cdefBF537.h $(VDSP)/Blackfin/include/cdefBF534.h $(VDSP)/Blackfin/include/cdef_LPBlackfin.h $(VDSP)/Blackfin/include/cplus/new $(VDSP)/Blackfin/include/cplus/exception $(VDSP)/Blackfin/include/cplus/xstddef $(VDSP)/Blackfin/include/xsyslock.h $(VDSP)/Blackfin/include/sys/adi_rtl_sync.h $(VDSP)/Blackfin/include/stdbool.h $(VDSP)/Blackfin/include/stdint.h $(VDSP)/Blackfin/include/cplus/cstdlib $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/cplus/cstddef $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/math.h $(VDSP)/Blackfin/include/ymath.h $(VDSP)/Blackfin/include/math_bf.h $(VDSP)/Blackfin/include/fract_typedef.h $(VDSP)/Blackfin/include/fract_math.h $(VDSP)/Blackfin/include/ccblkfn.h $(VDSP)/Blackfin/include/builtins.h $(VDSP)/Blackfin/include/sys/builtins_support.h $(VDSP)/Blackfin/include/fr2x16_typedef.h $(VDSP)/Blackfin/include/r2x16_typedef.h $(VDSP)/Blackfin/include/raw_typedef.h $(VDSP)/Blackfin/include/sys/anomaly_macros_rtl.h $(VDSP)/Blackfin/include/sys/mc_typedef.h $(VDSP)/Blackfin/include/fr2x16_math.h $(VDSP)/Blackfin/include/fr2x16_base.h $(VDSP)/Blackfin/include/r2x16_base.h ../../../ext/Unity/src/unity.h ../../../ext/Unity/src/unity_internals.h $(VDSP)/Blackfin/include/setjmp.h $(VDSP)/Blackfin/include/stdio.h 
	@echo ".\MainThread.cpp"
	$(VDSP)/ccblkfn.exe -c .\MainThread.cpp -c++ -g -structs-do-not-overlap -no-multiline -I ../../../src -I ../../../ext\Unity\src -double-size-32 -decls-strong -warn-protos -threads -Wsuppress 0001,0815 -proc ADSP-BF537 -o .\Debug\MainThread.doj -MM

VDK.h VDK.cpp unit-test.rbld :$(VDSP)/blackfin/vdk/VDK.cpp.tf $(VDSP)/blackfin/vdk/VDK.h.tf $(VDSP)/blackfin/vdk/VDKGen.exe ./unit-test.vdk unit-test.rbld 
	@echo ".\unit-test.vdk"
	$(VDSP)/Blackfin\vdk\vdkgen.exe .\unit-test.vdk -proc ADSP-BF537 -MM

Debug/unity.doj :../../../ext/Unity/src/unity.c ../../../ext/Unity/src/unity.h ../../../ext/Unity/src/unity_internals.h $(VDSP)/Blackfin/include/setjmp.h $(VDSP)/Blackfin/include/yvals.h $(VDSP)/Blackfin/include/math.h $(VDSP)/Blackfin/include/ymath.h $(VDSP)/Blackfin/include/math_bf.h $(VDSP)/Blackfin/include/fract_typedef.h $(VDSP)/Blackfin/include/fract_math.h $(VDSP)/Blackfin/include/ccblkfn.h $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/builtins.h $(VDSP)/Blackfin/include/sys/builtins_support.h $(VDSP)/Blackfin/include/fr2x16_typedef.h $(VDSP)/Blackfin/include/r2x16_typedef.h $(VDSP)/Blackfin/include/raw_typedef.h $(VDSP)/Blackfin/include/sys/anomaly_macros_rtl.h $(VDSP)/Blackfin/include/sys/mc_typedef.h $(VDSP)/Blackfin/include/fr2x16_math.h $(VDSP)/Blackfin/include/fr2x16_base.h $(VDSP)/Blackfin/include/r2x16_base.h $(VDSP)/Blackfin/include/stddef.h $(VDSP)/Blackfin/include/stdint.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/stdio.h 
	@echo "..\..\..\ext\Unity\src\unity.c"
	$(VDSP)/ccblkfn.exe -c ..\..\..\ext\Unity\src\unity.c -g -structs-do-not-overlap -no-multiline -I ../../../src -I ../../../ext\Unity\src -double-size-32 -decls-strong -warn-protos -threads -Wsuppress 0001,0815 -proc ADSP-BF537 -o .\Debug\unity.doj -MM

Debug/VDK.doj :VDK.cpp VDK.h $(VDSP)/Blackfin/include/defBF537.h $(VDSP)/Blackfin/include/def_LPBlackfin.h $(VDSP)/Blackfin/include/defBF534.h $(VDSP)/Blackfin/include/VDK_Public.h $(VDSP)/Blackfin/include/limits.h $(VDSP)/Blackfin/include/yvals.h $(VDSP)/Blackfin/include/stddef.h $(VDSP)/Blackfin/include/assert.h $(VDSP)/Blackfin/include/sys/platform.h $(VDSP)/Blackfin/include/sys/_adi_platform.h $(VDSP)/Blackfin/include/cdefBF537.h $(VDSP)/Blackfin/include/cdefBF534.h $(VDSP)/Blackfin/include/cdef_LPBlackfin.h $(VDSP)/Blackfin/include/cplus/new $(VDSP)/Blackfin/include/cplus/exception $(VDSP)/Blackfin/include/cplus/xstddef $(VDSP)/Blackfin/include/xsyslock.h $(VDSP)/Blackfin/include/sys/adi_rtl_sync.h $(VDSP)/Blackfin/include/stdbool.h $(VDSP)/Blackfin/include/stdint.h $(VDSP)/Blackfin/include/cplus/cstdlib $(VDSP)/Blackfin/include/stdlib.h $(VDSP)/Blackfin/include/stdlib_bf.h $(VDSP)/Blackfin/include/cplus/cstddef $(VDSP)/Blackfin/include/string.h $(VDSP)/Blackfin/include/VDK_Internals.h $(VDSP)/Blackfin/include/sys/exception.h MainThread.h 
	@echo ".\VDK.cpp"
	$(VDSP)/ccblkfn.exe -c .\VDK.cpp -c++ -g -structs-do-not-overlap -no-multiline -I ../../../src -I ../../../ext\Unity\src -double-size-32 -decls-strong -warn-protos -threads -Wsuppress 0001,0815 -proc ADSP-BF537 -o .\Debug\VDK.doj -MM

./Debug/unit-test.dxe :./VDK-BF537.ldf ./VDK.h $(VDSP)/Blackfin/lib/bf534_rev_0.3/crtsfc532mty.doj ./Debug/ExceptionHandler-BF537.doj ./Debug/MainThread.doj ./Debug/VDK.doj ./Debug/unity.doj $(VDSP)/BLACKFIN/lib/cplbtab537.doj $(VDSP)/Blackfin/lib/bf534_rev_0.3/crtn532mty.doj $(VDSP)/Blackfin/lib/bf534_rev_0.3/TMK-BF532.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/vdk-CORE-BF532.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/vdk-i-BF532.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libsmall532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/__initsbsz532.doj $(VDSP)/Blackfin/lib/bf534_rev_0.3/libio532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libc532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/librt532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libevent532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libcpp532mty.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libf64ieee532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libdsp532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libsftflt532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/libetsi532y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/Debug/libosal532_vdky.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/Debug/libssl537_vdky.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/Debug/libdrv537y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/Debug/libusb537y.dlb $(VDSP)/Blackfin/lib/bf534_rev_0.3/idle532mty.doj $(VDSP)/Blackfin/lib/bf534_rev_0.3/librt_fileio532mty.dlb 
	@echo "Linking..."
	$(VDSP)/ccblkfn.exe .\Debug\ExceptionHandler-BF537.doj .\Debug\MainThread.doj .\Debug\unity.doj .\Debug\VDK.doj -T .\VDK-BF537.ldf -L .\Debug -add-debug-libpaths -flags-link -od,.\Debug -o .\Debug\unit-test.dxe -proc ADSP-BF537 -flags-link -MD_ADI_THREADS -flags-link -MM

endif

ifeq ($(MAKECMDGOALS),unit-test_Debug_clean)

unit-test_Debug_clean:
	-$(RM) ".\Debug\ExceptionHandler-BF537.doj"
	-$(RM) "Debug\MainThread.doj"
	-$(RM) "Debug\unity.doj"
	-$(RM) "Debug\VDK.doj"
	-$(RM) ".\Debug\unit-test.dxe"
	-$(RM) ".\Debug\*.ipa"
	-$(RM) ".\Debug\*.opa"
	-$(RM) ".\Debug\*.ti"
	-$(RM) ".\Debug\*.pgi"
	-$(RM) ".\*.rbld"

endif


