#include <apt-pkg/cachefile.h>
#include <apt-pkg/pkgcache.h>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

static bool is_installed(unsigned char state)
{
	return state != pkgCache::State::PkgCurrentState::NotInstalled
	    && state != pkgCache::State::PkgCurrentState::ConfigFiles;
}

void find_top_pkgs()
{
	pkgCacheFile cache_file;
	std::set < std::string > top_level_pkgs;

	for (auto package = cache_file.GetPkgCache()->PkgBegin();
	     !package.end(); package++) {
		if (!is_installed(package->CurrentState)) {
			continue;
		}
		bool top_level = true;
		for (auto rdeps = package.RevDependsList(); !rdeps.end();
		     rdeps++) {
			auto owner = rdeps.ParentPkg();
			if (is_installed(owner->CurrentState)) {
				top_level = false;
				break;
			}
		}
		if (top_level) {
			std::ostringstream qualified_name;
			qualified_name << package.
			    Name() << ":" << package.Arch();
			top_level_pkgs.insert(qualified_name.str());
		}
	}

	for (auto name = top_level_pkgs.begin(); name != top_level_pkgs.end();
	     name++) {
		std::cout << *name << std::endl;
	}

}

int main()
{
	if (!pkgInitConfig(*_config) || !pkgInitSystem(*_config, _system)) {
		_error->DumpErrors();
		return 1;
	}
	find_top_pkgs();
	delete _config;
	delete _error;
	return 0;
}
