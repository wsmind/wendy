rem for /R %%f in (*.js *.html) do echo %%f

curl -X PUT -T wendy.js http://localhost:46288/data/.tools/wendy.js
curl -X PUT -T test/wip.html http://localhost:46288/data/.tools/test/wip.html
