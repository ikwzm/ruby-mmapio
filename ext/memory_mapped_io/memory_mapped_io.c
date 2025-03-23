/*********************************************************************************
 *
 *       Copyright (C) 2025 Ichiro Kawazome
 *       All rights reserved.
 * 
 *       Redistribution and use in source and binary forms, with or without
 *       modification, are permitted provided that the following conditions
 *       are met:
 * 
 *         1. Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 * 
 *         2. Redistributions in binary form must reproduce the above copyright
 *            notice, this list of conditions and the following disclaimer in
 *            the documentation and/or other materials provided with the
 *            distribution.
 * 
 *       THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *       "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *       LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *       A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 *       OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *       SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *       LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *       DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *       THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *       (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *       OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 ********************************************************************************/
#include "ruby.h"
#include <stddef.h>

#ifndef MODULE_NAME
#define MODULE_NAME         MMapIO
#endif
#ifndef OBJECT_NAME
#define OBJECT_NAME         MemoryMappedIO
#endif

#define TO_STR(x)           #x
#define NAME_TO_STR(x)      TO_STR(x)
#define MODULE_NAME_STRING  NAME_TO_STR(MODULE_NAME)
#define OBJECT_NAME_STRING  NAME_TO_STR(OBJECT_NAME)

typedef struct {
    void*    addr;
    size_t   size;
} mmapio_object;

#if SIZEOF_VOIDP == SIZEOF_LONG
#define VALUE_TO_VOIDP(x)     ((void*)(NUM2ULONG(x)))
#else
#define VALUE_TO_VOIDP(x)     ((void*)(NUM2ULL(x)))
#endif
#if SIZEOF_SIZE_T == SIZEOF_LONG
#define VALUE_TO_SIZE_T(x)    ((size_t)(NUM2LONG(x)))
#else
#define VALUE_TO_SIZE_T(x)    ((size_t)(NUM2ULL(x)))
#endif
#if SIZEOF_PTRDIFF_T == SIZEOF_LONG
#define VALUE_TO_PTRDIFF_T(x) ((ptrdiff_t)(NUM2LONG(x)))
#else
#define VALUE_TO_PTRDIFF_T(x) ((ptrdiff_t)(NUM2ULL(x)))
#endif

#define DATA_T           uint64_t
#define VALUE_TO_DATA(x) ((DATA_T)(NUM2ULL(x)))
#define DATA_TO_VALUE(x) (ULL2NUM((DATA_T)(x)))

#define DEFINE_MMAPIO_CHECK_OFFSET_METHOD(type)                               \
static int mmapio_check_offset_ ## type(mmapio_object* this, ptrdiff_t offset)\
{                                                                             \
    if ((offset < 0) || (offset >= (ptrdiff_t)(this->size))) {                \
        rb_raise(rb_eIndexError, "Offset exceeds mapped region");             \
        return -1;                                                            \
    }                                                                         \
    if ((offset % sizeof(type)) != 0) {                                       \
        rb_raise(rb_eArgError, "Offset violates " #type " alignment");        \
        return -1;                                                            \
    }                                                                         \
    return 0;                                                                 \
}

DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint8_t )
DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint16_t)
DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint32_t)
DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint64_t)

#define DEFINE_MMAPIO_RW_METHOD(name,type)                                 \
static VALUE mmapio_read_ ## name(VALUE self, VALUE offset)                \
{                                                                          \
    mmapio_object* this;                                                   \
    ptrdiff_t      _offset = VALUE_TO_PTRDIFF_T(offset);                   \
    type           _data;                                                  \
    Data_Get_Struct(self, mmapio_object, this);                            \
    if (mmapio_check_offset_ ## type(this, _offset) != 0) {                \
        return Qnil;                                                       \
    }                                                                      \
    _data = *(type *)((uintptr_t)(this->addr) + _offset);                  \
    return DATA_TO_VALUE(_data);                                           \
}                                                                          \
static VALUE mmapio_write_ ## name(VALUE self, VALUE offset, VALUE data)   \
{                                                                          \
    mmapio_object* this;                                                   \
    ptrdiff_t      _offset = VALUE_TO_PTRDIFF_T(offset);                   \
    DATA_T         _data   = VALUE_TO_DATA(data);                          \
    DATA_T         _mask   = (DATA_T)(~(type)0);                           \
    Data_Get_Struct(self, mmapio_object, this);                            \
    if (mmapio_check_offset_ ## type(this, _offset) != 0) {                \
        return Qnil;                                                       \
    }                                                                      \
    *(type *)((uintptr_t)(this->addr) + _offset) = (type)(_data & _mask);  \
    return Qnil;                                                           \
}

DEFINE_MMAPIO_RW_METHOD(uint8 ,uint8_t )
DEFINE_MMAPIO_RW_METHOD(uint16,uint16_t)
DEFINE_MMAPIO_RW_METHOD(uint32,uint32_t)
DEFINE_MMAPIO_RW_METHOD(uint64,uint64_t)

static VALUE mmapio_allocate(VALUE klass)
{
    mmapio_object* this = ALLOC(mmapio_object);
    return Data_Wrap_Struct(klass, NULL, RUBY_DEFAULT_FREE, this);
}

static VALUE mmapio_initialize(VALUE self, VALUE addr, VALUE offset, VALUE size)
{
    mmapio_object* this;
    Data_Get_Struct(self, mmapio_object, this);

    this->addr = VALUE_TO_VOIDP(addr) + VALUE_TO_PTRDIFF_T(offset);
    this->size = VALUE_TO_SIZE_T(size);

    return self;
}

void Init_memory_mapped_io(void)
{
    VALUE module = rb_const_get(rb_cObject, rb_intern(MODULE_NAME_STRING));
    VALUE object = rb_define_class_under(module, OBJECT_NAME_STRING, rb_cObject);
    rb_define_alloc_func(object, mmapio_allocate);
    rb_define_method(object, "initialize"  , mmapio_initialize  , 3);
    rb_define_method(object, "read_byte"   , mmapio_read_uint8  , 1);
    rb_define_method(object, "read_half"   , mmapio_read_uint16 , 1);
    rb_define_method(object, "read_word"   , mmapio_read_uint32 , 1);
    rb_define_method(object, "read_quad"   , mmapio_read_uint64 , 1);
    rb_define_method(object, "read_uint8"  , mmapio_read_uint8  , 1);
    rb_define_method(object, "read_uint16" , mmapio_read_uint16 , 1);
    rb_define_method(object, "read_uint32" , mmapio_read_uint32 , 1);
    rb_define_method(object, "read_uint64" , mmapio_read_uint64 , 1);
    rb_define_method(object, "write_byte"  , mmapio_write_uint8 , 2);
    rb_define_method(object, "write_half"  , mmapio_write_uint16, 2);
    rb_define_method(object, "write_word"  , mmapio_write_uint32, 2);
    rb_define_method(object, "write_quad"  , mmapio_write_uint64, 2);
    rb_define_method(object, "write_uint8" , mmapio_write_uint8 , 2);
    rb_define_method(object, "write_uint16", mmapio_write_uint16, 2);
    rb_define_method(object, "write_uint32", mmapio_write_uint32, 2);
    rb_define_method(object, "write_uint64", mmapio_write_uint64, 2);
}
