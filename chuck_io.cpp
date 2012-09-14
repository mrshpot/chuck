/*----------------------------------------------------------------------------
 ChucK Concurrent, On-the-fly Audio Programming Language
 Compiler and Virtual Machine
 
 Copyright (c) 2004 Ge Wang and Perry R. Cook.  All rights reserved.
 http://chuck.cs.princeton.edu/
 http://soundlab.cs.princeton.edu/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 -----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: chuck_io.cpp
// desc: chuck i/o
//
// authors: Spencer Salazar (spencer@ccrma.stanford.edu)
//    date: Summer 2012
//-----------------------------------------------------------------------------


#include "chuck_io.h"
#include "chuck_errmsg.h"
#include "util_serial.h"
#include "chuck_instr.h"
#include "chuck_type.h"
#include "chuck_globals.h"
#include "chuck_vm.h"

#include <poll.h>
#include <fcntl.h>
#include <termios.h>
#include <math.h>

#include <ext/stdio_filebuf.h>
#include <iostream>
#include <fstream>

using namespace std;

// available baud rates
const t_CKUINT Chuck_IO_Serial::BAUD_2400   = 2400;
const t_CKUINT Chuck_IO_Serial::BAUD_4800   = 4800;
const t_CKUINT Chuck_IO_Serial::BAUD_9600   = 9600;
const t_CKUINT Chuck_IO_Serial::BAUD_19200  = 19200;
const t_CKUINT Chuck_IO_Serial::BAUD_38400  = 38400;
const t_CKUINT Chuck_IO_Serial::BAUD_7200   = 7200;
const t_CKUINT Chuck_IO_Serial::BAUD_14400  = 14400;
const t_CKUINT Chuck_IO_Serial::BAUD_28800  = 28800;
const t_CKUINT Chuck_IO_Serial::BAUD_57600  = 57600;
const t_CKUINT Chuck_IO_Serial::BAUD_76800  = 76800;
const t_CKUINT Chuck_IO_Serial::BAUD_115200 = 115200;
const t_CKUINT Chuck_IO_Serial::BAUD_230400 = 230400;

const t_CKUINT Chuck_IO_Serial::TYPE_BYTE;
const t_CKUINT Chuck_IO_Serial::TYPE_WORD;
const t_CKUINT Chuck_IO_Serial::TYPE_INT;
const t_CKUINT Chuck_IO_Serial::TYPE_FLOAT;
const t_CKUINT Chuck_IO_Serial::TYPE_STRING;
const t_CKUINT Chuck_IO_Serial::TYPE_LINE;



void *Chuck_IO_Serial::shell_read_cb(void *_this)
{
    Chuck_IO_Serial *cereal = (Chuck_IO_Serial *) _this;
    
    cereal->read_cb();
    
    return NULL;
}


Chuck_IO_Serial::Chuck_IO_Serial() :
m_asyncReadRequests(CircularBuffer<Read>(32)),
m_asyncReadResponses(CircularBuffer<Read>(32))
{
    m_fd = -1;
    m_cfd = NULL;
    
    m_buf_size = 1024;
    m_buf = new char[m_buf_size];
    
    m_read_thread = NULL;
    m_event_buffer = NULL;
}

Chuck_IO_Serial::~Chuck_IO_Serial()
{
    m_do_read_thread = false;
    SAFE_DELETE(m_read_thread);
    if( m_event_buffer )
        g_vm->destroy_event_buffer( m_event_buffer );
    
    close();
    
    delete[] m_buf;
    m_buf = NULL;
    m_buf_size = 0;
}

t_CKBOOL Chuck_IO_Serial::ready()
{
    struct pollfd pollfds;
    pollfds.fd = m_fd;
    pollfds.events = POLLIN;
    pollfds.revents = 0;
    
    int result = poll(&pollfds, 1, 0);
    
    if( result > 0 && (pollfds.revents & POLLIN) ) return TRUE;
    else return FALSE; // TODO: error handling
}

t_CKBOOL Chuck_IO_Serial::open( const t_CKUINT i, t_CKINT flags, t_CKUINT baud )
{
    vector<string> ports = SerialIOManager::availableSerialDevices();
    
    if( flags & Chuck_IO_File::TYPE_BINARY )
    {
        m_flags = Chuck_IO_File::TYPE_BINARY;
    }
    else if( flags & Chuck_IO_File::TYPE_ASCII )
    {
        m_flags = Chuck_IO_File::TYPE_ASCII;
    }
    
    if( i < ports.size() )
    {
        const string &path = ports[i];
        int fd = ::open( path.c_str(), O_RDWR );
        if( fd >= 0 ) m_fd = fd;
        
        // set default baud rate
        setBaudRate(baud);
        
        m_cfd = fdopen(fd, "r+");
    }
    
    return TRUE;
}


t_CKBOOL Chuck_IO_Serial::open( const std::string & path, t_CKINT flags )
{
    return FALSE;
}


Chuck_String * Chuck_IO_Serial::readLine()
{
}


t_CKBOOL Chuck_IO_Serial::good()
{
    return m_fd >= 0;
}

void Chuck_IO_Serial::close()
{
    if(good())
    {
        if(m_flags & Chuck_IO_File::TYPE_ASCII)
            fclose(m_cfd);
        else
            ::close(m_fd);
        m_fd = -1;
        m_cfd = NULL;
    }
}

void Chuck_IO_Serial::flush()
{
    
}

t_CKINT Chuck_IO_Serial::mode()
{
    return MODE_ASYNC;
}

void Chuck_IO_Serial::mode( t_CKINT flag )
{
    // ...
}

// reading
t_CKINT Chuck_IO_Serial::readInt( t_CKINT flags )
{
    t_CKINT i = 0;
    
    if( flags ==  READ_INT8 )
    {
        
    }
}

t_CKFLOAT Chuck_IO_Serial::readFloat()
{
    
}

t_CKBOOL Chuck_IO_Serial::readString( std::string & str )
{
    
}

t_CKBOOL Chuck_IO_Serial::eof()
{
    
}


// writing
void Chuck_IO_Serial::write( const std::string & val )
{
    
}

void Chuck_IO_Serial::write( t_CKINT val )
{
    
}

void Chuck_IO_Serial::write( t_CKFLOAT val )
{
    
}


void Chuck_IO_Serial::readAsync( t_CKUINT type, t_CKUINT num )
{
    if(m_read_thread == NULL)
    {
        m_read_thread = new XThread();
        m_read_thread->start(shell_read_cb, this);
        
        assert(m_event_buffer == NULL);
        m_event_buffer = g_vm->create_event_buffer();
    }
    
    Read read;
    read.m_type = type;
    read.m_num = num;
    read.m_val = 0;
    read.m_status = Read::STATUS_PENDING;
    
    if(!m_asyncReadRequests.atMaximum() )
    {
        m_asyncReadRequests.put(read);
    }
    else
    {
        EM_log(CK_LOG_SEVERE, "SerialIO.readAsync: error: request buffer overflow, dropping read");
    }
}

Chuck_String * Chuck_IO_Serial::getLine()
{
    Read r;
    
    Chuck_String * str = NULL;

    m_asyncReadResponses.peek(r, 1);
    if(r.m_type == TYPE_LINE && r.m_status == Read::STATUS_SUCCESS)
    {
        str = (Chuck_String *) r.m_val;
        m_asyncReadResponses.get(r);
    }
    
    return str;
}


t_CKINT Chuck_IO_Serial::getByte()
{
    Read r;
    
    t_CKINT i = 0;
    
    m_asyncReadResponses.peek(r, 1);
    if(r.m_type == TYPE_BYTE && r.m_num == 1 &&
       r.m_status == Read::STATUS_SUCCESS)
    {
        i = (t_CKINT) r.m_val;
        m_asyncReadResponses.get(r);
    }
    
    return i;
}

Chuck_Array * Chuck_IO_Serial::getBytes()
{
    Read r;
    
    Chuck_Array * arr = NULL;
    
    m_asyncReadResponses.peek(r, 1);
    if(r.m_type == TYPE_BYTE && r.m_num > 1 &&
       r.m_status == Read::STATUS_SUCCESS)
    {
        arr = (Chuck_Array *) r.m_val;
        initialize_object(arr, &t_array);
        m_asyncReadResponses.get(r);
    }
    
    return arr;
}

Chuck_Array * Chuck_IO_Serial::getInts()
{
    Read r;
    
    Chuck_Array * arr = NULL;
    
    m_asyncReadResponses.peek(r, 1);
    if( r.m_type == TYPE_INT && r.m_status == Read::STATUS_SUCCESS)
    {
        arr = (Chuck_Array *) r.m_val;
        initialize_object(arr, &t_array);
        m_asyncReadResponses.get(r);
    }
    
    return arr;
}

Chuck_Array * Chuck_IO_Serial::getFloats()
{
    Read r;
    
    Chuck_Array * arr = NULL;
    
    m_asyncReadResponses.peek(r, 1);
    if( r.m_type == TYPE_FLOAT && r.m_status == Read::STATUS_SUCCESS)
    {
        arr = (Chuck_Array *) r.m_val;
        initialize_object(arr, &t_array);
        m_asyncReadResponses.get(r);
    }
    
    return arr;
}

Chuck_String * Chuck_IO_Serial::getString()
{
    
}


t_CKBOOL Chuck_IO_Serial::handle_line(Chuck_IO_Serial::Read &r)
{
    if(fgets(m_buf, m_buf_size, m_cfd))
    {
        int len = strlen(m_buf);
        // truncate end-of-line \n or \r's
        for(int i = len - 1; i > 0; i--)
        {
            if(m_buf[i] == '\n' || m_buf[i] == '\r')
                m_buf[i] = '\0';
            else
                break;
        }
        
        Chuck_String * str = new Chuck_String;
        str->str = std::string(m_buf);
        
        r.m_val = (t_CKUINT) str;
        r.m_status = Chuck_IO_Serial::Read::STATUS_SUCCESS;
    }
    else
    {
        r.m_val = 0;
        r.m_status = Chuck_IO_Serial::Read::STATUS_FAILURE;
    }
    
    return TRUE;
}

t_CKBOOL Chuck_IO_Serial::handle_string(Chuck_IO_Serial::Read & r)
{
}

t_CKBOOL Chuck_IO_Serial::handle_float_ascii(Chuck_IO_Serial::Read & r)
{
    t_CKFLOAT val = 0;
    int numRead = 0;
    Chuck_Array8 * array = new Chuck_Array8(0);
    for(int i = 0; i < r.m_num; i++)
    {
        if(fscanf(m_cfd, "%lf", &val))
        {
            numRead++;
            array->push_back(val);
        }
        else
        {
            // SPENCERTODO: error?
            break;
        }
    }
    
    r.m_num = numRead;
    r.m_val = (t_CKUINT) array;
    r.m_status = Read::STATUS_SUCCESS;
    
    return TRUE;
}

t_CKBOOL Chuck_IO_Serial::handle_int_ascii(Chuck_IO_Serial::Read & r)
{
    t_CKINT val = 0;
    int numRead = 0;
    Chuck_Array4 * array = new Chuck_Array4(FALSE, 0);
    for(int i = 0; i < r.m_num; i++)
    {
        if(fscanf(m_cfd, "%li", &val))
        {
            numRead++;
            array->push_back(val);
        }
        else
        {
            // SPENCERTODO: error
            break;
        }
    }
    
    r.m_num = numRead;
    r.m_val = (t_CKUINT) array;
    r.m_status = Read::STATUS_SUCCESS;
    
    return TRUE;
}

t_CKBOOL Chuck_IO_Serial::handle_byte(Chuck_IO_Serial::Read & r)
{
    // binary
    int size = 1;
    
    int num = r.m_num;
    
    if(size*num > m_buf_size)
    {
        int new_num = (int) floorf(((float)m_buf_size)/size);
        EM_log(CK_LOG_WARNING, "SerialIO.read_cb: error: request size %i too large (%i bytes), truncating to %i",
               num, size*num, new_num);
        num = new_num;
    }
    
    r.m_num = fread(m_buf, size, num, m_cfd);
    
    t_CKUINT val = 0;
    
    if(r.m_num == 1)
        val = m_buf[0];
    else
    {
        Chuck_Array4 * array = new Chuck_Array4(FALSE, r.m_num);
        for(int i = 0; i < r.m_num; i++)
        {
            array->set(i, m_buf[i]);
        }
        
        val = (t_CKUINT) array;
    }
    
    r.m_val = val;
    r.m_status = Read::STATUS_SUCCESS;
    
    return TRUE;
}

t_CKBOOL Chuck_IO_Serial::handle_float_binary(Chuck_IO_Serial::Read & r)
{
    // binary
    int size = 4; // SPENCERTODO: should these be based on arch (64 bit)?
    assert(sizeof(t_CKSINGLE) == 4);
    
    int num = r.m_num;
    
    if(size*num > m_buf_size)
    {
        int new_num = (int) floorf(((float)m_buf_size)/size);
        EM_log(CK_LOG_WARNING, "SerialIO.read_cb: error: request size %i too large (%i bytes), truncating to %i",
               num, size*num, new_num);
        num = new_num;
    }
    
    r.m_num = fread(m_buf, size, num, m_cfd);
    
    t_CKUINT val = 0;
    t_CKSINGLE * m_floats = (t_CKSINGLE *) m_buf;
    
    Chuck_Array8 * array = new Chuck_Array8(r.m_num);
    for(int i = 0; i < r.m_num; i++)
    {
        array->set(i, m_floats[i]);
    }
        
    val = (t_CKUINT) array;
    
    r.m_val = val;
    r.m_status = Read::STATUS_SUCCESS;
    
    return TRUE;
}

t_CKBOOL Chuck_IO_Serial::handle_int_binary(Chuck_IO_Serial::Read & r)
{
    // binary
    int size = 4; // SPENCERTODO: should these be based on arch (64 bit)?
    
    int num = r.m_num;
    
    if(size*num > m_buf_size)
    {
        int new_num = (int) floorf(((float)m_buf_size)/size);
        EM_log(CK_LOG_WARNING, "SerialIO.read_cb: error: request size %i too large (%i bytes), truncating to %i",
               num, size*num, new_num);
        num = new_num;
    }
    
    r.m_num = fread(m_buf, size, num, m_cfd);
    
    t_CKUINT val = 0;
    uint32_t * m_ints = (uint32_t *) m_buf;
    
    Chuck_Array4 * array = new Chuck_Array4(FALSE, r.m_num);
    for(int i = 0; i < r.m_num; i++)
    {
        array->set(i, m_ints[i]);
    }
    
    val = (t_CKUINT) array;
    
    r.m_val = val;
    r.m_status = Read::STATUS_SUCCESS;
    
    return TRUE;
}


void Chuck_IO_Serial::read_cb()
{
    m_do_read_thread = true;
    
    while(m_do_read_thread)
    {
        Read r;
        int num_responses = 0;
        
        while(m_asyncReadRequests.get(r))
        {
            if( m_asyncReadResponses.atMaximum() )
            {
                EM_log(CK_LOG_SEVERE, "SerialIO.read_cb: error: response buffer overflow, dropping read");
                continue;
            }
            
            if(m_flags & Chuck_IO_File::TYPE_ASCII)
            {
                switch(r.m_type)
                {
                    case TYPE_LINE:
                        handle_line(r);
                        break;
                        
                    case TYPE_STRING:
                        handle_string(r);
                        break;
                        
                    case TYPE_INT:
                        handle_int_ascii(r);
                        break;
                        
                    case TYPE_FLOAT:
                        handle_float_ascii(r);
                        break;
                        
                    default:
                        // this shouldnt happen
                        r.m_type = TYPE_NONE;
                        r.m_num = 0;
                        r.m_status = Read::STATUS_INVALID;
                        r.m_val = 0;
                        EM_log(CK_LOG_WARNING, "SerialIO.read_cb: error: invalid request");
                }
            }
            else
            {
                // binary
                switch(r.m_type)
                {
                    case TYPE_BYTE:
                        handle_byte(r);
                        break;
                    case TYPE_INT:
                        handle_int_binary(r);
                        break;
                    case TYPE_FLOAT:
                        handle_float_binary(r);
                        break;
                        
                    default:
                        // this shouldnt happen
                        r.m_type = TYPE_NONE;
                        r.m_num = 0;
                        r.m_status = Read::STATUS_INVALID;
                        r.m_val = 0;
                        EM_log(CK_LOG_WARNING, "SerialIO.read_cb: error: invalid request");
                }
            }
            
            m_asyncReadResponses.put(r);
            num_responses++;
        }
        
        if(num_responses)
            queue_broadcast(m_event_buffer);
        
        usleep(100);
    }
}


t_CKBOOL Chuck_IO_Serial::setBaudRate( t_CKUINT rate )
{
    // set serial IO baud rate
    struct termios tios;
    tcgetattr(m_fd, &tios);
    cfsetispeed(&tios, rate);
    cfsetospeed(&tios, rate);
    tcsetattr(m_fd, TCSAFLUSH, &tios);
    
    return TRUE;
}


t_CKUINT Chuck_IO_Serial::getBaudRate()
{
    // set serial IO baud rate
    struct termios tios;
    tcgetattr(m_fd, &tios);
    speed_t rate = cfgetispeed(&tios);
    
    return rate;
}


//-----------------------------------------------------------------------------
// serialio API
//-----------------------------------------------------------------------------
CK_DLL_SFUN( serialio_list );
CK_DLL_ALLOC( serialio_alloc );
CK_DLL_CTOR( serialio_ctor );
CK_DLL_DTOR( serialio_dtor );
CK_DLL_MFUN( serialio_open );
CK_DLL_MFUN( serialio_ready );
CK_DLL_MFUN( serialio_readLine );
CK_DLL_MFUN( serialio_onLine );
CK_DLL_MFUN( serialio_onByte );
CK_DLL_MFUN( serialio_onBytes );
CK_DLL_MFUN( serialio_onInts );
CK_DLL_MFUN( serialio_onFloats );
CK_DLL_MFUN( serialio_getLine );
CK_DLL_MFUN( serialio_getByte );
CK_DLL_MFUN( serialio_getBytes );
CK_DLL_MFUN( serialio_getInts );
CK_DLL_MFUN( serialio_setBaudRate );
CK_DLL_MFUN( serialio_getBaudRate );


//-----------------------------------------------------------------------------
// name: init_class_serialio()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_serialio( Chuck_Env * env )
{
    // init as base class
    Chuck_DL_Func * func = NULL;
    
    // log
    EM_log( CK_LOG_SEVERE, "class 'SerialIO'" );
    
    Chuck_Type * type = type_engine_import_class_begin( env, "SerialIO", "IO",
                                                     env->global(), serialio_ctor, serialio_dtor );
    // TODO: ctor/dtor?
    if( !type )
        return FALSE;
    
    // HACK; SPENCERTODO: better way to set this
    type->allocator = serialio_alloc;
    
    // add list()
    func = make_new_sfun( "string[]", "list", serialio_list );
    if( !type_engine_import_sfun( env, func ) ) goto error;
    
    func = make_new_mfun("int", "open", serialio_open);
    func->add_arg("int", "i");
    func->add_arg("int", "baud");
    func->add_arg("int", "mode");
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add ready (data available)
    func = make_new_mfun("int", "dataAvailable", serialio_ready);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add readLine
    func = make_new_mfun("string", "readLine", serialio_readLine);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add onLine
    func = make_new_mfun("SerialIO", "onLine", serialio_onLine);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add onByte
    func = make_new_mfun("SerialIO", "onByte", serialio_onByte);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add onBytes
    func = make_new_mfun("SerialIO", "onBytes", serialio_onBytes);
    func->add_arg("int", "num");
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add onInts
    func = make_new_mfun("SerialIO", "onInts", serialio_onInts);
    func->add_arg("int", "num");
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add onInts
    func = make_new_mfun("SerialIO", "onFloats", serialio_onFloats);
    func->add_arg("int", "num");
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add getLine
    func = make_new_mfun("string", "getLine", serialio_getLine);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add getByte
    func = make_new_mfun("int", "getByte", serialio_getByte);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add getBytes
    func = make_new_mfun("int[]", "getBytes", serialio_getBytes);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add getInts
    func = make_new_mfun("int[]", "getInts", serialio_getInts);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add setBaudRate
    func = make_new_mfun("int", "baudRate", serialio_setBaudRate);
    func->add_arg("int", "r");
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add getBaudRate
    func = make_new_mfun("int", "baudRate", serialio_getBaudRate);
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    // add baud rate constants
    type_engine_import_svar(env, "int", "B2400",   TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_2400);
    type_engine_import_svar(env, "int", "B4800",   TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_4800);
    type_engine_import_svar(env, "int", "B9600",   TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_9600);
    type_engine_import_svar(env, "int", "B19200",  TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_19200);
    type_engine_import_svar(env, "int", "B38400",  TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_38400);
    type_engine_import_svar(env, "int", "B7200",   TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_7200);
    type_engine_import_svar(env, "int", "B14400",  TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_14400);
    type_engine_import_svar(env, "int", "B28800",  TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_28800);
    type_engine_import_svar(env, "int", "B57600",  TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_57600);
    type_engine_import_svar(env, "int", "B76800",  TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_76800);
    type_engine_import_svar(env, "int", "B115200", TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_115200);
    type_engine_import_svar(env, "int", "B230400", TRUE, (t_CKUINT) &Chuck_IO_Serial::BAUD_230400);

    type_engine_import_svar(env, "int", "BINARY", TRUE, (t_CKUINT) &Chuck_IO_File::TYPE_BINARY);
    type_engine_import_svar(env, "int", "ASCII", TRUE, (t_CKUINT) &Chuck_IO_File::TYPE_ASCII);

    // end the class import
    type_engine_import_class_end( env );
    
    return TRUE;
    
error:
    
    // end the class import
    type_engine_import_class_end( env );
    
    return FALSE;
}



CK_DLL_SFUN( serialio_list )
{
    vector<string> devices = SerialIOManager::availableSerialDevices();
    
    // ISSUE: 64-bit
    Chuck_Array4 * array = new Chuck_Array4(TRUE, 0);
    initialize_object( array, &t_array );
    
    for(vector<string>::iterator i = devices.begin(); i != devices.end(); i++)
    {
        Chuck_String * name = new Chuck_String(*i);
        initialize_object(name, &t_string);
        array->push_back((t_CKUINT) name);
    }
    
    RETURN->v_object = array;
}


CK_DLL_ALLOC( serialio_alloc )
{
    return new Chuck_IO_Serial;
}

CK_DLL_CTOR( serialio_ctor )
{ }

CK_DLL_DTOR( serialio_dtor )
{ }

CK_DLL_MFUN( serialio_open )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    t_CKINT num = GET_NEXT_INT(ARGS);
    t_CKINT baud = GET_NEXT_INT(ARGS);
    t_CKINT mode = GET_NEXT_INT(ARGS);
    
    RETURN->v_int = cereal->open(num, mode, baud);
}

CK_DLL_MFUN( serialio_ready )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    
    RETURN->v_int = cereal->ready();
}

CK_DLL_MFUN( serialio_readLine )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    
    RETURN->v_string = cereal->readLine();
}


CK_DLL_MFUN( serialio_onLine )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    
    cereal->readAsync(Chuck_IO_Serial::TYPE_LINE, 1);
    
    RETURN->v_object = cereal;
}


CK_DLL_MFUN( serialio_onByte )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    
    cereal->readAsync(Chuck_IO_Serial::TYPE_BYTE, 1);
    
    RETURN->v_object = cereal;
}


CK_DLL_MFUN( serialio_onBytes )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    
    t_CKINT num = GET_NEXT_INT(ARGS);
    
    cereal->readAsync(Chuck_IO_Serial::TYPE_BYTE, num);
    
    RETURN->v_object = cereal;
}


CK_DLL_MFUN( serialio_onInts )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    
    t_CKINT num = GET_NEXT_INT(ARGS);
    
    cereal->readAsync(Chuck_IO_Serial::TYPE_INT, num);
    
    RETURN->v_object = cereal;
}


CK_DLL_MFUN( serialio_onFloats )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    
    t_CKINT num = GET_NEXT_INT(ARGS);
    
    cereal->readAsync(Chuck_IO_Serial::TYPE_FLOAT, num);
    
    RETURN->v_object = cereal;
}


CK_DLL_MFUN( serialio_getLine )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    RETURN->v_string = cereal->getLine();
}


CK_DLL_MFUN( serialio_getByte )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    RETURN->v_int = cereal->getByte();
}


CK_DLL_MFUN( serialio_getBytes )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    RETURN->v_object = cereal->getBytes();
}


CK_DLL_MFUN( serialio_getInts )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    RETURN->v_object = cereal->getInts();
}


CK_DLL_MFUN( serialio_setBaudRate )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    cereal->setBaudRate(GET_NEXT_INT(ARGS));
    RETURN->v_int = cereal->getBaudRate();
}


CK_DLL_MFUN( serialio_getBaudRate )
{
    Chuck_IO_Serial * cereal = (Chuck_IO_Serial *) SELF;
    RETURN->v_int = cereal->getBaudRate();
}

