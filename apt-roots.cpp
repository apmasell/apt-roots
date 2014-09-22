#include <apt-pkg/cachefile.h>
#include <apt-pkg/pkgcache.h>
#include <iostream>
#include <set>
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
	std::set < std::string > provide_pkgs;

	for (auto package = cache_file.GetPkgCache()->PkgBegin();
	     !package.end(); package++) {
		if (!is_installed(package->CurrentState)) {
			continue;
		}
		for (auto rdeps = package.RevDependsList(); !rdeps.end();
		     rdeps++) {
			if (rdeps.TargetVer() == NULL) {
				provide_pkgs.insert(rdeps.
						    ParentPkg().FullName(true));
			}
		}
	}

	for (auto package = cache_file.GetPkgCache()->PkgBegin();
	     !package.end(); package++) {
		if (!is_installed(package->CurrentState)) {
			continue;
		}
		bool top_level = true;
		for (auto rdeps = package.RevDependsList();
		     top_level && !rdeps.end(); rdeps++) {
			auto owner = rdeps.ParentPkg();
			if (is_installed(owner->CurrentState)) {
				top_level = false;
			}
		}
		for (auto provides = package.ProvidesList();
		     top_level && !provides.end(); provides++) {
			if (provides.OwnerPkg().Name() ==
			    provides.ParentPkg().Name()) {
				continue;
			}
			auto provides_name = provides.OwnerPkg().FullName(true);
			if (provide_pkgs.find(provides_name) !=
			    provide_pkgs.end()) {
				top_level = false;
			}
		}
		if (top_level) {
			top_level_pkgs.insert(package.FullName(true));
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
