#!/usr/bin/env bash
set -e

for type in Release Debug; do
	for introspect in OFF ON; do
		for objectmanager in OFF ON; do
			tag="debug"
			[[ $type = Release ]] && tag="release"
			[[ $introspect = ON ]] && tag="$tag+introspect"
			[[ $objectmanager = ON ]] && tag="$tag+objectmanager"

			echo "Building configuration '$tag'"
			build_dir=build/$tag
			if [ ! -d $build_dir ]; then
				mkdir -p $build_dir
				pushd $build_dir
				cmake -GNinja -DCMAKE_BUILD_TYPE=$type -DHAVE_INTROSPECTION=$introspect -DHAVE_OBJECT_MANAGER=$objectmanager  ../..
				popd
			fi

			pushd $build_dir
			ninja
			popd
		done
	done
done

