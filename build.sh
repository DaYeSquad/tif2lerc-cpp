ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# make & install liblerc

cd ""${ROOT_DIR}"/vendor/lerc/src"

cmake .

make install

# make & install tif2lerc

cd "${ROOT_DIR}"

cmake .

make install
