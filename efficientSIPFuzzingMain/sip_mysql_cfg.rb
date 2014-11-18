$SIP_MYSQL = {
	# "Method" => ["SIP-Method"],
	"Request-URI" => ["SIP-Request-URI-SIP", "SIP-Request-URI-User", "SIP-Request-URI-Host", "SIP-Version"],
	"Via" => ["SIP-Via", "SIP-Via-Host", "SIP-Via-Hostcolon", "SIP-Via-Hostport", "SIP-Via-Version", "SIP-Via-Tag"],
	"From" => ["SIP-From", "SIP-From-Displayname", "SIP-From-Tag", "SIP-From-Colon", "SIP-From-URI-SIP", "SIP-From-URI-User", "SIP-From-URI-Host"],
	"Contact" => ["SIP-Contact", "SIP-Contact-Displayname", "SIP-Contact-URI", "SIP-Contact-Left-Paranthesis", "SIP-Contact-Right-Paranthesis"],
	"To" => ["SIP-To", "SIP-To-Displayname", "SIP-To-URI", "SIP-To-Tag", "SIP-To-Left-Paranthesis", "SIP-To-Right-Paranthesis"],
	"Call-Id" => ["SIP-Call-Id", "SIP-Call-Id-Value", "SIP-Call-Id-At", "SIP-Call-Id-Ip"],
	"Expires" => ["SIP-Expires"],
	"Max-Forwards" => ["SIP-Max-Forwards"],
	"Cseq" => ["SIP-Cseq", "SIP-Cseq-Integer", "SIP-Cseq-String"],
	"Content-Type" => ["SIP-Content-Type", "SIP-Content-Type-App", "SIP-Content-Type-Prto"],
	"Content-Length" => ["SIP-Content-Length"],
	"Request-CRLF" => ["SIP-Request-CRLF"]
}

# CRLF-Request
# SDP-Attribute-CRLF
# SDP-Proto-v
# SDP-Proto-v-Identifier
# SDP-Proto-v-Equal
# SDP-Proto-v-Integer
# SDP-Origin
# SDP-Origin-Username
# SDP-Origin-Sessionid
# SDP-Origin-Networktype
# SDP-Origin-Ip
# SDP-Session
# SDP-Connection
# SDP-Connection-Networktype
# SDP-Connection-Ip
# SDP-Time
# SDP-Time-Start
# SDP-Time-Stop
# SDP-Media
# SDP-Media-Media
# SDP-Media-Port
# SDP-Media-Transport
# SDP-Media-Type
# SDP-Attribute
# SDP-Attribute-Rtpmap
# SDP-Attribute-Colon
# SDP-Attribute-Payloadtype
# SDP-Attribute-Encodingname
# SDP-Attribute-Slash
# SDP-Attribute-Clockrate
