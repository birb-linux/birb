#!/bin/sh
aur_dir="$HOME/programs/aur"

# Handle arguments
case $1 in
	"")
		echo -n "Search: "
		read query
	;;

	*)
		query="$1"
	;;
esac


package_name=$(curl -s "https://aur.archlinux.org/rpc/?v=5&type=search&arg=$query" | jq -r ".results[].Name" | fzf)
[ -z $package_name ] && exit 0
PKGBUILD_URL=$(curl -s "https://aur.archlinux.org/rpc/?v=5&type=info&arg[]=$package_name" | jq -r ".results[0].URLPath")

pkgbuild_path="$aur_dir/PKGBUILD"
[ -d "$pkgbuild_path" ] || mkdir -p "$pkgbuild_path"
wget -q -O $pkgbuild_path/$package_name.tar.gz https://aur.archlinux.org$PKGBUILD_URL

[ -d "$pkgbuild_path/$package_name" ] && rm -rvf "$pkgbuild_path/$package_name"
tar -xf $pkgbuild_path/$package_name.tar.gz --directory $pkgbuild_path/
rm -v $pkgbuild_path/$package_name.tar.gz

# Handle the PKGBUILD
PKGBUILD="$pkgbuild_path/$package_name/PKGBUILD"
#_pkgname=$(awk '/^_pkgname/' $PKGBUILD | cut -d'=' -f2)
#pkgname=$(awk '/^pkgname/' $PKGBUILD | cut -d'=' -f2 | sed "s/\"//g; s/\${_pkgname}/$_pkgname/g")
#_basever=$(awk '/^_basever/' $PKGBUILD | cut -d'=' -f2)
#_subver=$(awk '/^_subver/' $PKGBUILD | cut -d'=' -f2)
#arch=$(awk '/^arch/' $PKGBUILD | cut -d'=' -f2 | sed "s/('//g; s/')//g")
#
#pkgver=$(awk '/^pkgver=/' $PKGBUILD | cut -d'=' -f2 | sed "s/\$_basever/$_basever/g; s/\$_subver/$_subver/g; s/\"//g")
#_target=$(awk '/^_target/' $PKGBUILD | cut -d'=' -f2 | sed "s/\$_basever/$_basever/g; s/\$_subver/$_subver/g; s/\"//g")
while IFS= read -r line; do
case $line in
	_pkgname*)
		_pkgname=${line#_pkgname=}
		echo "_pkgname: $_pkgname"
		;;
	_basever*)
		_basever=${line#_basever=}
		echo "_basever: $_basever"
		;;
	_subver*)
		_subver=${line#_subver=}
		echo "_subver: $_subver"
		;;
	arch*)
		arch=$(echo "${line#arch=}" | tr -d "()'")
		echo "arch: $arch"
		;;
	pkgver*)
		pkgver=${line#pkgver=}
		echo "pkgver: $pkgver"
		;;
	_target*)
		_target=${line#_target=}
		echo "_target: $_target"
		;;
esac 
done < $PKGBUILD



function source_clean()
{
	source=$(sed "s/\$[{]_subver[}]/$_subver/g" <<< $source)
	source=$(sed "s/\$[{]_basever[}]/$_basever/g" <<< $source)
	source=$(sed "s/\$[{]_target[}]/$_target/g" <<< $source)
	source=$(sed "s/\$[{]pkgver[}]/$pkgver/g" <<< $source)
	source=$(sed "s/\$[{]_pkgname[}]/$_pkgname/g" <<< $source)
	source=$(sed "s/\$[{]pkgname[}]/$pkgname/g" <<< $source)
	source=$(sed "s/\$[{]pkgname[}]/$pkgname/g" <<< $source)
	source=$(sed "s/\$[{]arch[}]/$arch/g" <<< $source)
	source=$(sed "s/git+//g" <<< $source)
	source="$(sed "s/^.*http/http/g; s/')//g; s/('//g" <<< $source)"
}

function single_line_source()
{
	source=$(awk '/^source/' $PKGBUILD | cut -d'=' -f2 | sed "s/(\"//g; s/\".*//g; s/\"//g")
}

function multiline_source()
{
	echo "Multiline source variable detected. Getting the first one..."
	source=$(grep -A 1 -E '^source' $PKGBUILD | sed "/^source/d; s/^[[:space:]]*//g; s/\"//g")
}

[ $(grep -E '^source=\($' $PKGBUILD) ] && multiline_source || single_line_source
source_clean

echo -e "\n> \e[1mSource:\e[0m $source"
echo -n "How to handle the source? [g]it, [w]get, [c]ancel: "
read answer

source_dir="$aur_dir/packages/$pkgname"

case $answer in
	g|G)
		[ -d $source_dir ] || mkdir -p $source_dir

		git clone "$source" "$source_dir/${pkgname}"
		mv $PKGBUILD $source_dir
		;;

	w|W)
		[ -d $source_dir ] || mkdir -p $source_dir

		[ $(grep -i ".tar.gz" <<< $source) ] && output_file="$source_dir/${pkgname}_${pkgver}.tar.gz" || \
			output_file="$source_dir/${pkgname}_${pkgver}"

		wget -q -O $output_file "$source"
		tar -xf $output_file --directory $source_dir/
		rm -v $output_file
		mv $PKGBUILD $source_dir
		;;

	c|*)
		echo "Cancelled"
		;;
esac

# Do some cleanup
echo "Cleaning up..."
[ -d "$pkgbuild_path/$package_name" ] && rm -rf "$pkgbuild_path/$package_name"
