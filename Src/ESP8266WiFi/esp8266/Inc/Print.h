/*
 Print.h - Base class that provides print() and println()
 Copyright (c) 2008 David A. Mellis.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef Print_h
#define Print_h

#include <stdint.h>
#include <stddef.h>

#include "WString.h"
#include "Printable.h"

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print {
    private:
        int write_error;
        size_t printNumber(unsigned long, uint8_t);
        size_t printFloat(double, uint8_t);
    protected:
        void setWriteError(int err = 1) {
            write_error = err;
        }
    public:
        Print() :
                write_error(0) {
        }

        int getWriteError() {
            return write_error;
        }
        void clearWriteError() {
            setWriteError(0);
        }

        virtual size_t write(uint8_t) = 0;
        size_t write(const char *str) {
            if(str == NULL)
                return 0;
            return write((const uint8_t *) str, strlen(str));
        }
        virtual size_t write(const uint8_t *buffer, size_t size);
        size_t write(const char *buffer, size_t size) {
            return write((const uint8_t *) buffer, size);
        }
        // These handle ambiguity for write(0) case, because (0) can be a pointer or an integer
        size_t write(short t) { return write((uint8_t)t); }
        size_t write(unsigned short t) { return write((uint8_t)t); }
        size_t write(int t) { return write((uint8_t)t); }
        size_t write(unsigned int t) { return write((uint8_t)t); }
        size_t write(long t) { return write((uint8_t)t); }
        size_t write(unsigned long t) { return write((uint8_t)t); }

        // size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));


        virtual void flush() { /* Empty implementation for backward compatibility */ }
};

#endif
