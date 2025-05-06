curl -x http://localhost:8080 \
     --http1.0 -i \
     -H "Host: github.com" \
     -H "Cache-Control: no-cache" \
     http://github.com/
