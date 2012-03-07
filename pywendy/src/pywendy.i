%module(directors="1") pywendy
%{
#include <wendy/Asset.hpp>
#include <wendy/AssetFile.hpp>
#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>
#include <wendy/ProjectFileSystem.hpp>
%}

/* allows callbacks through inheritance */
%feature("director") wendy::ProjectListener;

%include "cdata.i"
%include "std_string.i"
%include "std_vector.i"

namespace std {
   %template(StringVector) vector<string>;
};

%include <wendy/common.hpp>
%include <wendy/Asset.hpp>
%include <wendy/AssetFile.hpp>
%include <wendy/Project.hpp>
%include <wendy/ProjectListener.hpp>
%include <wendy/ProjectFileSystem.hpp>
