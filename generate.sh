#!/bin/bash

shopt -s nullglob

HIGHLIGHT="highlight -O latex --replace-quotes -f -t 4 --encoding=utf-8 --syntax=cpp"
TOOL_ROOT=/home/git-repos/odtgen.local

declare -A src_dirs
pwd="$(pwd)"

for file in $@; do
	echo "Processing: ${file}"
	dir=$(dirname "${file}")
	src_dir="${dir}"/src
	if [ -d "${src_dir}" ] && [ -z "${src_dirs[${src_dir}]}" ]; then
		echo "Syntax highlighting for directory: ${src_dir}"
		src_dirs[${src_dir}]=1
		find "${src_dir}" -name '*.cpp' -exec /bin/bash -c "${HIGHLIGHT}"' -i "${0}" > "${0}".tex' {} \;
	fi

	cp -r "${TOOL_ROOT}"/workspace .
	title="$(grep '^\\title' "${file}" | sed 's;^\\title{\(.*\)}$;\1;')"
	sed -i 's_<dc:title>.*</dc:title>_<dc:title>'"${title}"'</dc:title>_' workspace/meta.xml
	(cd "${dir}" && exec "${TOOL_ROOT}"/odtgen < "$(basename ${file})" > "${pwd}"/workspace/content.main.xml 2> /dev/null)
	cat "${TOOL_ROOT}"/slim_xml/content.header.xml \
		workspace/content.main.xml \
		"${TOOL_ROOT}"/slim_xml/content.footer.xml > workspace/content.xml
	rm -f workspace/content.main.xml
	cp "${TOOL_ROOT}"/slim_xml/styles.xml workspace/
	cd workspace
	zip -r "${pwd}/$(basename "${file}" tex)"odt ./* > /dev/null
	cd ..
 	rm -rf workspace
done
