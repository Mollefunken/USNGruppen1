#!/bin/sh
POST_STRING=$(cat)

#echo $POST_STRING>test.xml
dbpath=/usr/local/apache2/db/Dikt.db

check_login () {
	sesjonsid=$(echo $HTTP_COOKIE | cut -d "=" -f 2)
	echo "select epostadresse from Sesjon where sesjonsID='$sesjonsid';" | sqlite3 $dbpath
}

logg_inn () {
	echo "select fornavn from Bruker where epostadresse='$1' AND passordhash='$2';" | sqlite3 $dbpath
}

logg_inn_session () {
	echo "INSERT INTO sesjon (sesjonsID, epostadresse) VALUES ('$1', '$2');" | sqlite3 $dbpath
}

test_funksjon () {
        pk=$(echo -n "pragma table_info("$1");" | sqlite3 $dbpath)
        echo $pk
}

get_primary_key () {

        while read data; do
        pk=$(echo $data | rev | cut -d "|" -f 1)
                if [ "$pk" = "1" ]; then
                        felt=$(echo $data | cut -d "|" -f 2)
                        echo $felt
                fi
        done
}

sql_to_xml () {

        while read data; do
                if [ -z "$data" ]; then
                        echo "    </"$1">"
                else
                        if [ -z "$prev" ]; then
                                echo "    <"$1">"
                        fi
                        columnname=$(echo $data | cut -d " " -f 1)
                        value=$(echo $data | cut -d " " -f 3-)
                        echo "      <"$columnname">"$value"</"$columnname">"
                fi
                prev=$data
        done

}

database=$(echo $REQUEST_URI | cut -d "/" -f 2)
table=$(echo $REQUEST_URI | cut -d "/" -f 3)
field=$(echo $REQUEST_URI | cut -d "/" -f 4)



if [ "$REQUEST_METHOD" = "GET" ]; then
	  echo "Content-type:text/xml;charset=utf-8"
	  echo
	  echo '<?xml version="1.0"?>'

	  echo '<root
	  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
          xsi:noNamespaceSchemaLocation="http://158.248.21.214/schema.xsd">'


          echo "  <"$database">"

          if [ -z "$field" ]; then

                echo -n 'select * from '$table';' | sqlite3 $dbpath -line | sql_to_xml $table

          else
                felt=$(echo 'pragma table_info('$table');' | sqlite3 $dbpath | get_primary_key)
                echo -n "select * from "$table" where "$felt"='"$field"';" | sqlite3 $dbpath -line | sql_to_xml $table
          fi
          echo "    </"$table">"
          echo "  </"$database">"
	  echo "</root>"
fi



if [ "$REQUEST_METHOD" = "POST" ]; then
	sesjonsid=$(echo $HTTP_COOKIE | cut -d "=" -f 2)

	echo "Content-type: text/plain;charset=utf-8"
	echo

	case $table in
		sesjon)
			epostadresse=$(echo $POST_STRING | xmlstarlet sel -t -v "//epostadresse" -n)
			pwd=$(echo $POST_STRING | xmlstarlet sel -t -v "//passordhash" -n)
			pwdhash=$(echo $pwd | sha1sum | cut -d ' ' -f1)

			loggedin=$(check_login)
			if [ -z "$loggedin" ]; then
				innlogg=$(logg_inn $epostadresse $pwdhash)
				if [ -z "$innlogg" ]; then
					echo "1"
				else
					logg_inn_session $sesjonsid $epostadresse
					echo "2"
				fi
			else
				echo "3"
			fi
			;;
		Dikt)
			epostadresse=$(check_login)
			if [ -z "$epostadresse" ]; then
				echo "1"
			else
				unencoded=$(echo $POST_STRING | xmlstarlet sel -t -v "//dikt" -n)
				dikt=$(echo $unencoded | sed 's/+/ /g')
				echo "insert into Dikt (dikt, epostadresse) values ('$dikt', '$epostadresse');" | sqlite3 $dbpath
				echo "2"
			fi
			;;
	esac
fi


if [ "$REQUEST_METHOD" = "PUT" ]; then
	sesjonsid=$(echo $HTTP_COOKIE | cut -d "=" -f 2)

	echo "Content-type: text/plain;charset=utf-8"
	echo
	epostadresse=$(check_login)
	if [ -z "$epostadresse" ]; then
		echo "1"
	else
		dikteier=$(echo "select epostadresse from dikt where diktID='$field';" | sqlite3 $dbpath)
		currentUser=$(echo "select epostadresse from sesjon where sesjonsid='$sesjonsid';" | sqlite3 $dbpath)

		if [ "$dikteier" = "$currentUser" ]; then
			unencoded=$(echo $POST_STRING | xmlstarlet sel -t -v "login/Dikt/dikt" -n)
	                dikt=$(echo $unencoded | sed 's/+/ /g')
			echo "update dikt set dikt='$dikt' where diktid='$field';" | sqlite3 $dbpath
		else
			echo "Dette er ikke ditt dikt"
		fi
	fi
fi

if [ "$REQUEST_METHOD" = "DELETE" ]; then
	echo "Content-type: text/plain;charset=utf-8"
	echo "Access-Control-Allow-Origin: http://158.248.21.214/"
	echo "Access-Control-Allow-Credentials: true"
	echo "Access-Control-Allow-Methods: POST,PUT,DELETE,GET"
        echo

	epostadresse=$(check_login)
	if [ -z "$epostadresse" ]; then
		var=$(echo "Du er ikke logget inn")
	else
		case $table in
			sesjon)
				sesjonsid=$(echo $HTTP_COOKIE | cut -d "=" -f 2)
				echo "delete from sesjon where sesjonsid='$sesjonsid';" | sqlite3 $dbpath
				;;
			Dikt)
				if [ -z "$field" ]; then
					echo "delete from dikt where epostadresse='$epostadresse';" | sqlite3 $dbpath
				else
					dikteier=$(echo "select epostadresse from dikt where diktid='$field';" | sqlite3 $dbpath)
					if [ "$dikteier" = "$epostadresse" ]; then
						echo "delete from dikt where diktid='$field';" | sqlite3 $dbpath
					#else
						#echo "Dette er ikke ditt dikt"
					fi
				fi
		esac
	fi
fi


if [ "$REQUEST_METHOD" = "OPTIONS" ]; then
	echo	

fi
