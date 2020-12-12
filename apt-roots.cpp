#include <apt-pkg/cachefile.h>
#include <apt-pkg/error.h>
#include <apt-pkg/init.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/pkgsystem.h>
#include <iostream>
#include <set>
#include <string>

void find_top_pkgs(void);

static bool is_installed(unsigned char state)
{
	return state != pkgCache::State::NotInstalled
	    && state != pkgCache::State::ConfigFiles;
}

void find_top_pkgs(void)
{
	pkgCacheFile cache_file;
	std::set < std::string > top_level_pkgs;
	std::set < std::string > provide_pkgs;

	if (!cache_file.Open(nullptr, false)) {
		_error->DumpErrors();
		return;
	}
	auto cache = cache_file.GetDepCache();

	/*
	 * Collect all the packages that are depended upon by installed packages but
	 * are “provides” package (i.e., there is no real package with the same name).
	 */
	for (auto package = cache->PkgBegin(); !package.end(); package++) {
		if (!is_installed(package->CurrentState)) {
			continue;
		}
		for (auto rdeps = package.RevDependsList(); !rdeps.end();
		     rdeps++) {
			if (rdeps.TargetVer() == nullptr) {
				provide_pkgs.insert(rdeps.
						    ParentPkg().FullName(true));
			}
		}
	}

	/*
	 * Filter through all the install packages.
	 */
	for (auto package = cache->PkgBegin(); !package.end(); package++) {
		if (!is_installed(package->CurrentState)) {
			continue;
		}
		bool top_level =
		    ((*cache)[package].Flags & pkgCache::Flag::Auto) == 0;

		/*
		 * This package is not top-level if another package depends on it, for any
		 * reason, including “suggests” or “recommends”.
		 */
		for (auto rdeps = package.RevDependsList();
		     top_level && !rdeps.end(); rdeps++) {
			auto owner = rdeps.ParentPkg();
			if (is_installed(owner->CurrentState)) {
				top_level = false;
			}
		}
		/*
		 * This package is not top-level if it “provides” a name that has been
		 * previously established as useful.
		 */
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
		/*
		 * Collect it.
		 */
		if (top_level) {
			top_level_pkgs.insert(package.FullName(true));
		}
	}

	/*
	 * Show the list, sorted alphabetically by the set.
	 */
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
	return 0;
}
