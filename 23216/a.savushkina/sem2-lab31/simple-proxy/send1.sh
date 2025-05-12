
curl -x http://localhost:65430 \
     --http1.0 -i \
     -H "Host: lib.ru" \
     -H "Cache-Control: max-age=120" \
     http://lib.ru/
