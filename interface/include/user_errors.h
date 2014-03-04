#ifndef _USER_ERRORS_H_
#define _USER_ERRORS_H_

#include <string>		// std::string

/*!
 * \brief Static Class for handling user errors
 * This static class handles all possible errors that can be generated by the user. Such errors include incorret calls
 * of CARLsim core functions, conceptual/chronological errors in setting up a network, etc.
 * All these errors are supposed to be fatal; that is, an error message will be printed and CARLsim will exit.
 */
class UserErrors {
public:
	// +++++ PUBLIC PROPERTIES ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

	/*!
	 * \brief an enum of all possible error codes
	 * This enum contains all the different error codes that are possible (in alphabetical order).
	 * Whenever an error is thrown, it has to be assigned one of these values.
	 * There is also type UNKNOWN for unspecified errors.
	 */
	enum errorType {
		ALL_NOT_ALLOWED,		//!< keyword ALL is not allowed for this variable
		CANNOT_BE_NEGATIVE,		//!< parameter cannot have negative value (opposite to "must be", but includes zero)
		CANNOT_BE_NULL,			//!< parameter cannot have NULL value
		CANNOT_BE_POSITIVE,		//!< parameter cannot have positive value (opposite to "must be", but includes zero)
		FILE_CANNOT_CREATE,		//!< could not create a file
		FILE_CANNOT_OPEN,		//!< could not open file
		MUST_BE_LOGGER_CUSTOM,	//!< must be in custom logger mode
		MUST_BE_NEGATIVE,		//!< parameter must have negative value
		MUST_BE_POSITIVE,		//!< parameter must have positive value
		MUST_HAVE_SAME_SIGN,	//!< some parameters must have the same sign
		NETWORK_ALREADY_RUN,	//!< function cannot be called because network has already bun run
		UNKNOWN_GROUP_ID,		//!< the specified group id is unknown
		UNKNOWN,				//!< an unknown error
		WRONG_NEURON_TYPE		//!< function cannot be applied to neuron type
	};


	// +++++ PUBLIC METHODS +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

	/*!
	 * \brief simple wrapper for assert statement
	 * This function evaluates a certain statement to true or false, and if it false, it will throw an error.
	 * \param[in] statement					the logical statement to evaluate
	 * \param[in] errorIfAssertionFails		the type of error to throw if assertion fails (from enum errorType)
	 * \param[in] errorFunc					a string to indicate the location where the error occured
	 */
	static void userAssert(bool statement, errorType errorIfAssertionFails, std::string errorFunc,
								std::string errorMsgPrefix="");

//	static void userAssertNonZero();
//	static void userAssertNonNegative();

private:
	// +++++ PRIVATE METHODS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

	/*!
	 * \brief simple wrapper for displaying an error message
	 * This function generates a standard error message for a certain error type, displays the message, and aborts
	 * the process.
	 * \param[in] errorFunc 	a string to indicate the location where the error occurred
	 * \param[in] error 		the type of error to throw (from enum errorType)
	 */
	static void throwError(std::string errorFunc, errorType error, std::string errorMsgPrefix="");
};

#endif