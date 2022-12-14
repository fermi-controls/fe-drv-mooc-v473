VID = 1.6
PRODUCT = 1

SUPPORTED_VERSIONS = 64 67

MOD_TARGETS = v473.out v473-dan.out
MOD_64_TARGETS = v473-cube.out
MOD_67_TARGETS = v473-cube.out

include ${PRODUCTS_INCDIR}frontend-3.1.mk

v473.o cube.o mooc_class.o test_v473.o : v473.h

v473.out : v473.o mooc_class.o ${PRODUCTS_LIBDIR}libvwpp-3.0.a
	${make-mod}

v473-cube.out : cube.o ${PRODUCTS_LIBDIR}libvwpp-3.0.a
	${make-mod}

v473-dan.out : test_v473.o ${PRODUCTS_LIBDIR}libvwpp-3.0.a
	${make-mod}
