/**
 * \brief Configuration file reader.
 * \author Jeremy Decker
 * \version 0.1
 * \date 31/09/2009
 */

#pragma once
#ifndef __REKCEDYMEREJ_CFG_HPP__
#define __REKCEDYMEREJ_CFG_HPP__

class Cfg
{
	public:
		Cfg();
		~Cfg();

		/**
		 * \brief Load configuration file.
		 * \param filename Path of configuration file to read.
		 */
		void load(const char * filename);

		/**
		 * \brief Set property value.
		 * \param key Property key.
		 * \param value Property value.
		 */
		void setValue (const char * key, const char * value);

		/**
		 * \brief Get property value.
		 * \param key Property key.
		 * \return value Property value or NULL if property not found.
		 */
		const char * getValue (const char * key);

		/**
		 * \brief Clear all properties.
		 */
		void   clear();

	private:
		char * * name;
		char * * value;
		int size;
};

#endif
