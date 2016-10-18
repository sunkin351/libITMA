/*
 * UnknownException.h
 *
 *  Created on: Oct 17, 2016
 *      Author: joshua
 */
#pragma once
#ifndef SRC_UNKNOWN_EXCEPTION_H_
#define SRC_UNKNOWN_EXCEPTION_H_

#include <exception>

class UnknownException : std::exception
{
public:
	virtual const char* what()
	{
		return "An Unknown Error Happened.";
	}
};



#endif /* SRC_UNKNOWN_EXCEPTION_H_ */
