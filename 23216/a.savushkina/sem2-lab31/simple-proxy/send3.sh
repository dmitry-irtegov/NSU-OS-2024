curl -x http://localhost:8080 \
     --http1.0 -i \
     -H "Host: google.com" \
     -H "Cache-Control: max-age=120" \
     http://google.com/
