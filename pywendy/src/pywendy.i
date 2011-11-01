%module(directors="1") pywendy
%{
#include <wendy/Asset.hpp>
#include <wendy/AssetFile.hpp>
#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>
%}

/* allows callbacks through inheritance */
%feature("director") wendy::ProjectListener;

%include <wendy/common.hpp>
%include <wendy/Asset.hpp>
%include <wendy/AssetFile.hpp>
%include <wendy/Project.hpp>
%include <wendy/ProjectListener.hpp>
