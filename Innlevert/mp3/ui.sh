#!/bin/sh


make_xml()
{
  echo '<?xml version="1.0" encoding ="utf-8"?>'
  echo '<login
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:noNamespaceSchemaLocation="http://158.248.21.214/schema.xsd\">'



  case $1 in
    login)
     # echo '<?xml version="1.0" encoding ="utf-8"?>'
     # echo '<login
#	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
#	xsi:noNamespaceSchemaLocation="http://158.248.21.214/schema.xsd\">'
      echo '<bruker>'
      echo '<epostadresse>'$2'</epostadresse>'
      echo '<passordhash>'$3'</passordhash>'
      echo '</bruker>'
     # echo '</login> '
    ;;
    addPoem)
 #       echo '<?xml version="1.0" encoding ="utf-8"?>'
#	echo '<root
 #	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  #	xsi:noNamespaceSchemaLocation="http://158.248.21.214/schema.xsd\">'
  	echo '<Dikt>'
	echo '<dikt>'$2'</dikt>'
	echo '</Dikt>'
    ;;
  esac
  echo '</login> '
}

if [ -z "$HTTP_COOKIE" ]; then
    cookie_value=$(pwgen -s 9 1)
    echo "Set-Cookie: sesjon=$cookie_value"
fi

sesjonsID=$(echo $HTTP_COOKIE | cut -d "=" -f 2)
response=$(wget -qO- -S --header="Cookie: $HTTP_Cookie" http://158.248.21.214:9090/Dikt/sesjon/$sesjonsID)
loginStatus=$(echo $response | xmlstarlet sel -t -v "//sesjonsID" -n)
status="Temp"
if [ "$sesjonsID" = "$loginStatus" ]; then
    status="Logget inn!"
else
    status="Du er IKKE Logget inn!"
fi

if [ "$REQUEST_METHOD" = "GET" ]; then
   # Skriver ut 'http-header' for 'plain-text'
    echo "Content-type:text/html;charset=utf-8"
    echo

    wget -qO- http://158.248.21.214/form.html | sed "s/validate/$status/"
fi



if [ "$REQUEST_METHOD" = "POST" ]; then

  read POST_STRING
  decode_post=$(printf '%b' "${POST_STRING//%/\\x}")

  name=$(echo -n $decode_post | cut -d '=' -f2 | cut -d '&' -f1)

  #echo "Content-type:text/html;charset=utf-8"
  #echo
  #echo $name

  case $name in

    getPoem)
      echo "Content-type:text/html;charset=utf-8"
      echo

      poemID=$(echo $decode_post | cut -d '=' -f3)
      RESPONSE=$(wget -qO- -S --header='Content-type:text/plain' http://158.248.21.214:9090/Dikt/dikt/$poemID)
      echo '<body>
      <center>'
      echo $RESPONSE | xmlstarlet tr --omit-decl http://158.248.21.214/poem.xsl
      echo    '</center>'
      echo  '<button onclick="window.location.href=`http://158.248.21.214:8080/ui.sh`">Go Back</button>'
      echo  '</body>'
      ;;

    addPoem)
      poem=$(echo $decode_post | cut -d '=' -f3)
      file=$(make_xml addPoem $poem)

      echo "Content-type:text/html;charset=utf-8"
      echo

      RESPONSE=$(wget -O- -S --header='Content-Type: text/xml' --header "Cookie: $HTTP_COOKIE" --post-data "$file" http://158.248.21.214:9090/Dikt/Dikt/)
      wget -qO- --header="Cookie: $HTTP_COOKIE" http://158.248.21.214:8080/ui.sh
      ;;

    changePoem)
      echo "Content-type:text/html;charset=utf-8"
      echo
      MidDiktID=$(echo $decode_post | cut -d '=' -f3)
      diktID=$(echo $MidDiktID | cut -d '&' -f1)
      dikt=$(echo $decode_post | cut -d '=' -f4)

      XML=$(make_xml addPoem $dikt)
      RESPONSE=$(wget -O- -S --header='Content-Type: text/xml' --header "Cookie: $HTTP_COOKIE" --method=PUT --body-data="$XML" http://158.248.21.214:9090/Dikt/Dikt/$diktID)
      wget -qO- --header="Cookie: $HTTP_COOKIE" http://158.248.21.214:8080/ui.sh
      ;;
    deletePoem)
      echo "Content-type:text/html;charset=utf-8"
      echo
      DiktID=$(echo $decode_post | cut -d '=' -f3)

      RESPONSE=$(wget -O- -S --header "Cookie: $HTTP_COOKIE" --method=DELETE http://158.248.21.214:9090/Dikt/Dikt/$DiktID)
      wget -qO- --header="Cookie: $HTTP_COOKIE" http://158.248.21.214:8080/ui.sh
      ;;
    showLogin)
      echo "Content-type: text/html"
      echo
      wget -O- -q http://158.248.21.214/login.html | sed "s/validate/Logg inn/"
      ;;

    login)
      email=$(echo $decode_post | cut -d '=' -f3 | cut -d '&' -f1)
      password=$(echo $decode_post | cut -d '=' -f4 | cut -d '&' -f1)
      #passhash=$(echo $password | sha1sum | cut -d ' ' -f1)

      echo "Content-type:text/html;charset=utf-8"
      echo

      XML=$(make_xml login $email $password)
      # echo $XML
      APIANSWER=$(wget -O- -S --header='Content-Type: text/xml' --header "Cookie: $HTTP_COOKIE" --post-data "$XML" http://158.248.21.214:9090/Dikt/sesjon/)

      case $APIANSWER in
      1)
     	wget -O- -q http://158.248.21.214/login.html | sed "s/validate/Invalid login/"
	;;
      2)
	echo  '<body>'
	echo  'Login successful'
	echo  '<button onclick="window.location.href=`http://158.248.21.214:8080/ui.sh`">Go Back</button>'
      	echo  '</body>'
	;;
      3)
	wget -O- -q http://158.248.21.214/login.html | sed "s/validate/Already logged inn/"
	;;
      esac
      ;;

    logout)
	echo "Content-type:text/html;charset=utf-8"
	echo

	RESPONSE=$(wget -O- -S --header "Cookie: $HTTP_COOKIE" --method=DELETE http://158.248.21.214:9090/Dikt/sesjon/)
	wget -qO- --header="Cookie: $HTTP_COOKIE" http://158.248.21.214:8080/ui.sh
	;;
 esac

fi


#if [ "$REQUEST_METHOD" = "PUT" ]; then
#    echo $REQUEST_URI skal endres slik:
#    echo

    # skriver-hode
#    head -c $CONTENT_LENGTH
#    echo 
#fi

#if [ "$REQUEST_METHOD" = "DELETE" ]; then

#RESPONSE=$(echo wget -O- -S --header='Content-Type: text/plain' --header "Cookie: $HTTP_COOKIE" --method=DELETE http://158.248.21.214:9090/Dikt/sesjon)
#echo $RESPONSE
#fi
