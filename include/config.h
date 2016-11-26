
// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/utility/string_view.hpp>
#include <string>

#include <daw/temp_file.h>

namespace daw {
	namespace glean {
		struct glean_config {
			boost::filesystem::path cache_folder; 
			std::string cmake_binary;
			std::string git_binary;
		public:
			glean_config( std::string CacheFolder, std::string cmake_binary_path, std::string git_binary_path );
			glean_config( boost::filesystem::path config_file );
			glean_config( );
			~glean_config( );
			glean_config( glean_config const & ) = default;
			glean_config( glean_config && ) = default;
			glean_config & operator=( glean_config const & ) = default;
			glean_config & operator=( glean_config && ) = default;

		};	// glean_config

		glean_config get_config( );

		/// @brief Download a file to a temporary location
		/// @param url Location of file to download
		/// @return path to downloaded file
		/// @post The file exists and will be deleted upon program termination
		daw::unique_temp_file download_file( boost::string_view url ); 
	}	// namespace glean
}    // namespace daw

