DIR="/opt/OpenIMSCore"
DIR_NAME="PCSCF_CA"

cd $DIR

echo Creating CA certificate
echo -----------------------
echo 1. create CA dir
        mkdir $DIR_NAME
        cd $DIR_NAME

echo "2. create ca dir structure and files  (see ca(1))"
	mkdir  private
        mkdir newcerts
        touch index.txt
        echo 01 >serial

echo 2. create CA private key
        openssl genrsa -out private/cakey.pem 2048
        chmod 600 private/cakey.pem

echo 3. create CA self-signed certificate
        openssl req -out cacert.pem   -x509 -new -key private/cakey.pem


echo Creating a server/client certificate
echo ------------------------------------
echo "1. create a certificate request (and its private key in privkey.pem)"
echo   WARNING: the organization name should be the same as in the ca certificate.
        openssl req -out pcscf_cert_req.pem -new -nodes
	cp privkey.pem pcscf_private_key.pem
	

echo 2. sign it with the ca certificate
	mkdir demoCA
	touch demoCA/index.txt
	cp serial demoCA/serial
        openssl ca -cert cacert.pem -keyfile private/cakey.pem -outdir . -in pcscf_cert_req.pem -out pcscf_cert.pem
	cat demoCA/index.txt >>index.txt
	rm -rf demoCA

echo Setting ser to use the certificate
echo ----------------------------------
echo 1. create the ca list file:
echo        for each of your ca certificates that you intend to use do:
                cat cacert.pem >>pcscf_ca_list.pem


