<?php
ob_start();
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=<code><span style="color: #000000">
<span style="color: #0000BB">&lt;?php
<br />highlight_file</span><span style="color: #007700">(</span><span style="color: #0000BB">__FILE__</span><span style="color: #007700">);
<br /></span><span style="color: #0000BB">$num&nbsp;</span><span style="color: #007700">=&nbsp;</span><span style="color: #0000BB">$_POST</span><span style="color: #007700">[</span><span style="color: #DD0000">'num'</span><span style="color: #007700">];
<br />if(isset(</span><span style="color: #0000BB">$num</span><span style="color: #007700">)){
<br />&nbsp;&nbsp;&nbsp;&nbsp;if(</span><span style="color: #0000BB">is_numeric</span><span style="color: #007700">(</span><span style="color: #0000BB">$num</span><span style="color: #007700">)){
<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span><span style="color: #0000BB">header</span><span style="color: #007700">(</span><span style="color: #DD0000">"Location:一些好康的"</span><span style="color: #007700">&nbsp;);</span><span style="color: #FF8000">//关注嘉然❤&nbsp;顿顿解馋
<br />&nbsp;&nbsp;&nbsp;&nbsp;</span><span style="color: #007700">}
<br />&nbsp;&nbsp;&nbsp;&nbsp;else{
<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if(</span><span style="color: #0000BB">$num&nbsp;</span><span style="color: #007700">==&nbsp;</span><span style="color: #0000BB">180</span><span style="color: #007700">){
<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;echo&nbsp;</span><span style="color: #0000BB">$flag</span><span style="color: #007700">;
<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
<br />&nbsp;&nbsp;&nbsp;&nbsp;}
<br />}</span>
</span>
</code>
    <title>嗨呀是我吗</title>
</head>
<body>
<?php
$num = $_POST['num'];
if(isset($num)){
    if(is_numeric($num)){
        header("Location:https://www.bilibili.com/video/BV1FX4y1g7u8" );
    }
    elseif($num == 180){
        echo "<strong>这里没有flag捏</strong>";
        setcookie("flag","ZJNUCTF{Guanzhu_Jiaran_Dundun_Jiechan}");
    	   echo "</br></br>"
;
        echo "HINT:草莓味曲奇饼干";

    }
}
?>
<div>
<img src="jiaran.gif" width="420px" height="420px">
</div>   
</body>
</html>