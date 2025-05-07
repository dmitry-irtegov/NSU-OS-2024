curl -x http://localhost:65430 \
     --http1.0 -i \
     -H "Host: helloworld.ru" \
     -H "Cache-Control: max-age=30" \
     http://helloworld.ru/
