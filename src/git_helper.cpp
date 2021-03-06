// The MIT License (MIT)
//
// Copyright (c) 2018-2019 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <string>
#include <vector>

#include "daw/glean/git_helper.h"
#include "daw/glean/utilities.h"

namespace daw::glean {
	std::vector<std::string>
	git_action_pull::build_args( fs::path const & ) const {
		return {"pull", "--ff-only"};
	}

	std::vector<std::string>
	git_action_clone::build_args( fs::path const &work_tree ) const {
		auto result = std::vector<std::string>{"clone"};
		if( recurse_submodules ) {
			result.emplace_back( "--recurse-submodules" );
		}
		result.push_back( remote_uri );
		result.push_back( work_tree.string( ) );
		return result;
	}

	std::vector<std::string>
	git_action_version::build_args( fs::path const & ) const {
		return {"checkout", version};
	}

	std::vector<std::string>
	git_action_reset::build_args( fs::path const & ) const {
		return {"reset", "--hard"};
	}
} // namespace daw::glean
