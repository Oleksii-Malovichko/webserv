#!/bin/bash


HOST="localhost"
PORT="8080"
TEST_FILE="test.txt"      
LOG_FILE="test.log"       
TOTAL_TESTS=0
PASSED_TESTS=0


RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' 


echo "Webserv Test Report - $(date)" > "$LOG_FILE"

# ========================
# ✅ curl test
# ========================
run_test() {
    local test_name="$1"
    local -a curl_args=("${@:2:$(($#-3))}") 
    local expect_code="${@: -2:1}"
    local expect_content="${@: -1}"

    if [[ -z "$expect_content" ]]; then
        echo -e "${RED}✗ Test '$test_name': Expect content cannot be empty${NC}"
        ((TOTAL_TESTS++))
        return 1
    fi

    local response
    local actual_code
    local exit_code
    
    response=$(curl -s -i "${curl_args[@]}" 2>&1)
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        echo -e "${RED}✗ Network error: curl exited with code $exit_code${NC}"
        return 1
    fi
    actual_code=$(curl -s -o /dev/null -w "%{http_code}" "${curl_args[@]}")

    {
        echo -e "\n===== Test: $test_name ====="
        echo "CURL Command: curl ${curl_args[*]}"
        echo -e "Response (exit code: $exit_code):\n$response"
    } >> "$LOG_FILE"

    ((TOTAL_TESTS++))
    local code_pass=0
    local content_pass=0

    if [[ "$actual_code" == "$expect_code" ]]; then
        code_pass=1

        if [[ "$actual_code" -ge 400 ]]; then
            content_pass=1 
        else
            if [[ "$response" == *"$expect_content"* ]]; then
                content_pass=1
            else
                echo -e "${RED}✗ Content missing: '$expect_content'${NC}"
            fi
        fi
    else
        echo -e "${RED}✗ Code error: expect $expect_code, actual $actual_code${NC}"
    fi

    if [[ $code_pass -eq 1 ]] && [[ $content_pass -eq 1 ]]; then
        echo -e "${GREEN}✓ $test_name passed${NC}"
        ((PASSED_TESTS++))
    else
        echo -e "${RED}✗ $test_name failed${NC}"
    fi
}

# ========================
# 🔥 nc request function
# ========================
run_nc_test() {
    local test_name="$1"
    local request="$2"
    local expect_code="$3"

    ((TOTAL_TESTS++))
    local response
    response=$(echo -e "$request" | nc -w 2 "$HOST" "$PORT" 2>/dev/null)

    local actual_code
    actual_code=$(echo "$response" | head -n 1 | awk '{print $2}')
    {
        echo -e "\n===== Test: $test_name ====="
        echo "Raw request sent:"
        echo -e "$request"
        echo "Response received:"
        echo -e "$response"
    } >> "$LOG_FILE"

    if [[ "$actual_code" == "$expect_code" ]]; then
        echo -e "${GREEN}✓ $test_name passed${NC}"
        ((PASSED_TESTS++))
    else
        echo -e "${RED}✗ $test_name failed${NC}"
    fi
}

# ========================
# 🚀 curl test
# ========================

echo -e "${YELLOW}\n************** curl check ********************${NC}"
echo -e "${YELLOW}**********************************************\n${NC}"

# test 1: simple GET method return index
run_test "GET method" \
  -s -i \
  "http://$HOST:$PORT/" \
  "200" \
  "$(cat resources/www/index.html)"

# test 2: simple GET method without index, autoindex
run_test "GET method fail" \
  -s -i \
  "http://$HOST:$PORT/assets" \
  "403" \
  "Not requested"

# test 3: Not implemented method
run_test "PUT method" \
  -s -i \
  -X PUT \
  "http://$HOST:$PORT/putfile" \
  "501" \
  "Not requested"

# test 4
run_test "POST chunked upload (.txt file)" \
  -s -i \
  -X POST \
  -F "file=@Makefile;type=text/plain" \
  -H "Transfer-Encoding: chunked" \
  "http://$HOST:$PORT/upload" \
  "201" \
  "File uploaded succesfully!"

# test 5
run_test "Same ip and port, different servername: webserv.com" \
  -s -i \
  "http://$HOST:$PORT/" \
  -H "Host: webserv.com" \
  "200" \
  "$(cat resources/www/index.html)"

# test 6
run_test "Same ip and port, different servername: portfolio.com" \
  -s -i \
  "http://$HOST:$PORT/" \
  -H "Host: portfolio.com" \
  "200" \
  "$(cat resources/www/index.html)"

# ========================
# 🔥 nc illegal test
# ========================

echo -e "${YELLOW}\n************** nc check **********************${NC}"
echo -e "${YELLOW}**********************************************\n${NC}"

echo -e "${YELLOW}\n>>>>>>>>>>>> Head error <<<<<<<<<<<\n${NC}"

run_nc_test "Missing Host Header" \
"GET / HTTP/1.1\r\n\r\n" \
"400"

run_nc_test "Illegal HTTP Method" \
"FORK / HTTP/1.1\r\nHost: localhost\r\n\r\n" \
"501"

run_nc_test "Bad HTTP Version" \
"GET / HTTP/2.0\r\nHost: localhost\r\n\r\n" \
"505"

run_nc_test "Missing CRLF" \
"GET / HTTP/1.1Host: localhost\r\n\r\n" \
"400"

run_nc_test "Garbage Request" \
"😈😈😈😈😈😈\r\n\r\n" \
"400"

echo -e "${YELLOW}\n>>>>>>>>>>>> GET <<<<<<<<<<<\n${NC}"

run_nc_test "bad request uri ." \
"GET . HTTP/1.1\r\nHost: localhost\r\n\r\n" \
"400"

run_nc_test "bad request uri not start with /" \
"GET index.html HTTP/1.1\r\nHost: localhost\r\n\r\n" \
"400"

run_nc_test "bad HTTP first line" \
"GET POST / HTTP/1.1\r\nHost: localhost\r\n\r\n" \
"400"

run_nc_test "HTTP full url without port" \
"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" \
"200"

run_nc_test "HTTP full url with port" \
"GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" \
"200"

run_nc_test "HTTP full url not patch" \
"GET http://localhost:8080/ HTTP/1.1\r\nHost: localhost\r\n\r\n" \
"400"

run_nc_test "GET HTTP Method with body" \
"GET / HTTP/1.1\r\nHost: localhost\r\n\r\nshuai" \
"200"

run_nc_test "GET HTTP Method with invalid uri" \
"GET /Hello World HTTP/1.1\r\nHost: localhost\r\n\r\n" \
"400"

echo -e "${YELLOW}\n>>>>>>>>>>>> POST <<<<<<<<<<<\n${NC}"

run_nc_test "POST without content-length or chunked" \
"POST /cgi-bin/showbody.php HTTP/1.1\r
Host: localhost\r\n\r\nshuai" \
"411"

run_nc_test "POST with both content-length and chunked" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\nshuai" \
"400"

run_nc_test "POST with exact content-length" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 5\r\n\r\nshuai" \
"200"

########################################
run_nc_test "POST without /upload" \
"POST /check HTTP/1.1\r\nHost: localhost\r
Content-Length: 5\r\n\r\nshuai" \
"403"
########################################

run_nc_test "POST with content-length too short" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 2\r\n\r\nshuai" \
"200"

run_nc_test "POST with content-length larger than max size" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 2000000\r\n\r\nshuai" \
"413"

run_nc_test "POST with content-length larger than max size, overflow" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 2147483648\r\n\r\nshuai" \
"400"

run_nc_test "POST with content-length 0" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r\n\r\nshuai" \
"200"

run_nc_test "POST with content-length negative" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: -1\r\n\r\nshuai" \
"400"

run_nc_test "POST with content-length begin with 0" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 005\r\n\r\nshuai" \
"400"

run_nc_test "POST with chunked 1" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Transfer-Encoding: chunked\r
Content-Type: text/plain\r\n\r
7\r\nMozilla\r\n9\r\nDeveloper\r\n7\r\nNetwork\r\n0\r\n\r\n" \
"200"

run_nc_test "POST with chunked 2" \
"POST /cgi-bin/showbody.php HTTP/1.1\r\nHost: localhost\r
Transfer-Encoding: chunked\r
Content-Type: text/plain\r\n\r
c\r\nMozilla\r\n; ;\r\n9\r\nDeveloper\r\n7\r\nNetwork\r\n0\r\n\r\n" \
"200"

echo -e "${YELLOW}\n>>>>>>>>>>>> SITE TEST <<<<<<<<<<<\n${NC}"

run_nc_test "GET /cgi/python/showenv.py, with pathinfo and querystring return autoindex" \
"GET /cgi/python/showenv.py/data/comment?userinfo=hello HTTP/1.1\r\nHost: localhost\r\n\r" \
"200"

run_nc_test "GET /" \
"GET / HTTP/1.1\r\nHost: localhost\r\n\r" \
"200"

run_nc_test "GET /data return autoindex" \
"GET /data HTTP/1.1\r\nHost: localhost\r\n\r" \
"200"

run_nc_test "GET /assets autoindex not allowed, return 403" \
"GET /assets HTTP/1.1\r\nHost: localhost\r\n\r" \
"403"

run_nc_test "GET /notexist, return 404" \
"GET /notexist HTTP/1.1\r\nHost: localhost\r\n\r" \
"404"

run_nc_test "GET /assets/notexist, return default 404" \
"GET /assets/notexist HTTP/1.1\r\nHost: localhost\r\n\r" \
"404"

run_nc_test "GET /getnotallowed, return default 405" \
"GET /getnotallowed HTTP/1.1\r\nHost: localhost\r\n\r" \
"405"

run_nc_test "DELETE /notexist, return default 404" \
"DELETE /notexist HTTP/1.1\r\nHost: localhost\r\n\r" \
"404"

run_nc_test "DELETE /assets, where DELETE not allowed, return default 405" \
"DELETE /assets HTTP/1.1\r\nHost: localhost\r\n\r" \
"405"

run_nc_test "DELETE /assets/1.png, where DELETE not allowed, return default 405" \
"DELETE /assets/1.png HTTP/1.1\r\nHost: localhost\r\n\r" \
"405"

mkdir -p ./site/web1/
touch ./site/web1/hello.test
run_nc_test "DELETE /hello.test, return 204" \
"DELETE /hello.test HTTP/1.1\r\nHost: localhost\r\n\r" \
"204"

mkdir -p ./site/web1/test/
touch ./site/web1/test/hello2.test
run_nc_test "DELETE /test/, return 204" \
"DELETE /test/ HTTP/1.1\r\nHost: localhost\r\n\r" \
"204"

run_nc_test "GET /redir, redirect to /data, return 308" \
"GET /redir HTTP/1.1\r\nHost: localhost\r\n\r" \
"308"

run_nc_test "GET /redirexample, redirect to example.com, return 307" \
"GET /redirexample HTTP/1.1\r\nHost: localhost\r\n\r" \
"307"

echo -e "${YELLOW}\n>>>>>>>>>>>> ERR TEST <<<<<<<<<<<\n${NC}"

run_nc_test "GET /a>{}, invalid uri character, return 400" \
"GET /a>{} HTTP/1.1\r\nHost: localhost\r\n\r" \
"400"

run_nc_test "GET with .. in uri, return 403" \
"GET /.. HTTP/1.1\r\nHost: localhost\r\n\r" \
"403"

run_nc_test "POST, repeat content length., return 400" \
"POST /upload HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
Content-Length: 1\r
\r\n\r" \
"400"

run_nc_test "POST, repeat content type., return 400" \
"POST /upload HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
Content-Type: text/html\r
Content-Type: text/html\r
\r\n\r" \
"400"

echo -e "${YELLOW}\n>>>>>>>>>>>> CGI ERR TEST <<<<<<<<<<<\n${NC}"

run_nc_test "POST script not exist, return 404" \
"POST /cgi/python/notexist.py HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
\r\n\r" \
"404"

run_nc_test "POST script contain error in stderr" \
"POST /cgi/python/error.py HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
\r\n\r" \
"502"

run_nc_test "POST script contain segmentation fault" \
"POST /cgi/python/crash.py HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
\r\n\r" \
"502"

run_nc_test "POST script return nothing" \
"POST /cgi/python/empty.py HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
\r\n\r" \
"502"

run_nc_test "POST script doesn't have content type" \
"POST /cgi/php/wrongCgiNoContentType.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
\r\n\r" \
"502"

run_nc_test "POST script have invalid head" \
"POST /cgi/php/wrongCgiInvalidHead.php HTTP/1.1\r\nHost: localhost\r
Content-Length: 0\r
\r\n\r" \
"502"

# ========================
# ✅ test log and statistic
# ========================
echo -e "\n${YELLOW}Tests completed: $PASSED_TESTS/$TOTAL_TESTS passed${NC}"
if [ $PASSED_TESTS -eq $TOTAL_TESTS ]; then
    echo -e "${GREEN}All tests passed!${NC}"
else
    echo -e "${RED} $(($TOTAL_TESTS - $PASSED_TESTS)) tests failed, check log file: $LOG_FILE${NC}"
fi