//Javascript
var wifi_json={};
var choos_wifiname='';
var pwsavestate=0;
var str_username="";
var str_userpass="";
var str_token="";
var str_wanType="";
var str_wanIp="";
var str_wanNetmask="";
var str_wifiname="";
var tocken_live;
/*
$(function(){

	$('.btn.close').click(function(e){
		e.preventDefault();
		$('#pwrequired').fadeOut();
	});
	$('.remove').click(function(e){
		e.preventDefault();
		$(this).parent('li').fadeOut(500);
	});

	$('#navigation li a').click(function(e){
		$(this).parent('li').addClass('current').siblings().removeClass('current');
	});
	

});
*/
//Javascript
function Page_click_OK()
{
	if(event.keyCode ==13)
	{
		if(location.hash == '#sys_login')
		{
			gettoken();
		}
		else if(location.hash == '#wifilist')
		{
			setwifi();
		}
		else if(location.hash == '#wire-setting')
		{
			change_wan();
		}
		else if(location.hash == '#wifi-setting')
		{
			w_change_wifi();
		}
		else if(location.hash == '#system-setting')
		{
			changepass();
		}
	}
}


$(document).on('click','#submit-close-wifi',function(e){
		$.ajax({ 
				type: "GET", 
				url: "/cgi-bin/close-wifi", 
		}); 
});


$(document).on('click','#change-internet',function(e){
	$.ajax({
	  type: 'POST',
	  async:false,
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","ifname"]}',
	  success: function(json) {
		if(json.result=="apcli0")
		{
			$.ajax({
			  type: 'POST',
			  async:false,
			  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
			  data: '{"method":"set","params":["network","wan","ifname","eth0.2"]}',
			  success: function(json) {
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"commit","params":["network"]}',
	  success: function(json) {

	  },
	  error: function(error) {
		alert(error);
		}
	});
	 			 },
			  error: function(error) {
				}
			});
		} 
		else
		{
			$.ajax({
			  type: 'POST',
			  async:false,
			  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
			  data: '{"method":"set","params":["network","wan","ifname","apcli0"]}',
			  success: function(json) {
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"commit","params":["network"]}',
	  success: function(json) {

	  },
	  error: function(error) {
		alert(error);
		}
	});
	 			 },
			  error: function(error) {
				}
			});
		}
	  },
	  error: function(error) {
		}
	});

	$.ajax({
	  type: 'POST',
	  async:false,
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","ifname"]}',
	  success: function(json) {
		if(json.result=="apcli0")
		{
			document.getElementById("setting_type").innerHTML="无线上网设置";
			document.getElementById("change-internet").value="切换到有线上网"
		} 
		else
		{
			document.getElementById("setting_type").innerHTML="有线上网设置";
			document.getElementById("change-internet").value="切换到无线上网"
		}
	  },
	  error: function(error) {
		}
	});


});


function setCookie(name,value,hours,path,domain,secure){
          var cdata = name + "=" + value;
          if(hours){
              var d = new Date();
              d.setHours(d.getHours() + hours);
              cdata += "; expires=" + d.toGMTString();
          }
          cdata +=path ? ("; path=" + path) : "" ;
          cdata +=domain ? ("; domain=" + domain) : "" ;
          cdata +=secure ? ("; secure=" + secure) : "" ;
          document.cookie = cdata;
      }
       
      function getCookie(name){
          var reg = eval("/(?:^|;\\s*)" + name + "=([^=]+)(?:;|$)/"); 
          return reg.test(document.cookie) ? RegExp.$1 : "";
      }
       
      function removeCookie(name,path,domain){
          this.setCookie(name,"",-1,path,domain);
      }


    function ifdisplay(){
    	$cur = $('.ifdisplay1').html();
		$('input').removeAttr('disabled').css('opacity','1');
    	switch($cur){
    		case '固定IP':
    			$('#pppoe-name,#pppoe-pw').attr('disabled','disabled').css('opacity','.3');
				$('#ip,#mask,#gate,#edit_DNS').removeAttr('disabled').css('opacity','1');
				if(str_token=="")
				{
					break;
				}
				getwanip();
				getwannetmask();
				getwangateway();
				getthePara('network','wan','dns','edit_DNS');
    			break;
    		case '动态获取':
				$('#pppoe-name,#pppoe-pw').attr('disabled','disabled').css('opacity','.3');
				$('#ip,#mask,#gate,#edit_DNS').attr('disabled','disabled').css('opacity','.3');
    			//$('.form>p input').attr('disabled','disabled').css('opacity','.3');
    			break;
    		case 'PPPOE':
    			$('#pppoe-name,#pppoe-pw').removeAttr('disabled').css('opacity','1');
    			$('#ip,#mask,#gate,#edit_DNS').attr('disabled','disabled').css('opacity','.3');
				if(str_token=="")
				{
					break;
				}
				getwanUsername();
				getthePara('network','wan','password','pppoe-pw');

    			break;
    		default:
    			$('input').removeAttr('disabled').css('opacity','1');
    			break;

    	}

    	$cur2 = $('.ifdisplay2').html();
    	switch($cur2){
    		case '不加密':
    			$('#wifi-pw').attr('disabled','disabled').css('opacity','.3');
    			break;
    		default:
    			$('#wifi-pw,#wifi').removeAttr('disabled').css('opacity','1');
    			break;

    	}
    }

function dosomething()
{
	var label = document.getElementById("ip_type_select");
	$.ajax({
	  type: 'POST',
	  async:false,
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","proto"]}',
	  success: function(json) {
		if(json.result=="dhcp")
		{
			label.innerHTML="动态获取";
		} 
		else if(json.result=="static")
		{
			label.innerHTML="固定IP";
		}
		else
		{
			label.innerHTML="PPPOE";
		}
		tocken_live=true;
	  },
	  error: function(error) {
		}
	});
	if(tocken_live)
	{
		location.hash = '#wifilist';
		getwifiname();
		getthePara_chanel('wireless','ra0','channel','select_chanel');
		//getthePara('wireless','ra0','channel','lable_channel');
		getthePara_rao2('wireless','ssid','ssid');
		getthePara_rao2('wireless','key','wifi-pw');
		get_pass_type('wireless','encryption','ip_pswtype_select');
		ifdisplay();
	}
	else
	{
		str_token="";
		location.hash = '#sys_login';
		document.getElementById("pswd_login").focus();
	}
}


$(function(){
	$(window).load(function(e){
		if(str_token=='')
		{
			
			
			if(getCookie("_token")=="")
			{
				location.hash = '#sys_login';
				document.getElementById("pswd_login").focus();
			}
			else
			{
				tocken_live=false;
				str_token=getCookie("_token");
				setTimeout("dosomething();",500 );
				
			}
		}
		
		
		

	});
	
	$('.wifilist li a').click(function(e){
		e.preventDefault();
		$('#pwrequired').fadeIn();
	});
	$('.btn.close').click(function(e){
		e.preventDefault();
		$('#pwrequired').fadeOut();
	});
	$('.remove').click(function(e){
		e.preventDefault();
		$(this).parent('li').fadeOut(500);
	});

	$('#navigation li a').click(function(e){
		$(this).parent('li').addClass('current').siblings().removeClass('current');
	});
	
	$('.has-menu li a').click(function(e){
		$('.has-menu').addClass('current').siblings().removeClass('current');
	});



	//select
	jQuery.fn.select = function(options){ 
        return this.each(function(){ 
            var $this = $(this); 
            var $shows = $this.find(".shows"); 
            var $selectOption = $this.find(".selectOption"); 
            var $el = $this.find("ul > li"); 
                                       
            $this.click(function(e){ 
                $(this).toggleClass("zIndex active"); 
                $(this).children("ul").toggleClass("dis"); 
                e.stopPropagation(); 
            }); 
               
            $el.bind("click",function(){ 
                var $this_ = $(this); 
                    
                $this.find("span").removeClass("gray"); 
                $this_.parent().parent().find(".selectOption").text($this_.text());
                //display or not
				ifdisplay();
            }); 
               
            $("body").bind("click",function(){ 
                $this.removeClass("zIndex"); 
                $this.find("ul").removeClass("dis");     
            }) 
               
        //eahc End   
        }); 
           
    } 
    $(".selectContainer").select();  


    
});

function commit(wireless)
{
	//alert('{"method":"commit","params":["'+wireless+'"]}');
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"commit","params":["'+wireless+'"]}',
	  success: function(json) {

	  },
	  error: function(error) {
		alert(error);
		}
	});
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/sys?auth="+str_token,
	  data: '{"method":"init.restart","params":["network"]}',
	  success: function(json) {
	  },
	  error: function(error) {
		alert(error);
		}
	});
}
function setthePara_rao(net,name,lable2)
{
	var label = document.getElementById(lable2);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	if(str_wifiname=="")
	{
		getwifiname();
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"set","params":["'+net+'","'+str_wifiname+'","'+name+'","'+label.value+'"]}',
	  success: function() {
		//commit(net);
	  },
	  error: function(error) {
		alert(error);
		}
	});
}
function setthePara_rao_value(net,name,value)
{
	//var label = document.getElementById(lable);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	if(str_wifiname=="")
	{
		getwifiname();
	}
	$.ajax({
	  type: 'POST',
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"set","params":["'+net+'","'+str_wifiname+'","'+name+'","'+value+'"]}',
	  success: function() {
		//commit(net);
	  },
	  error: function(error) {
		alert(error);
		}
	});
}
function setthePara(net,type,name,lable2)
{
	var label = document.getElementById(lable2);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"set","params":["'+net+'","'+type+'","'+name+'","'+label.value+'"]}',
	  success: function() {
		//commit(net);
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

function setthePara_value(net,type,name,value)
{
	//var label = document.getElementById(lable2);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"set","params":["'+net+'","'+type+'","'+name+'","'+value+'"]}',
	  success: function() {
		//commit(net);
	  },
	  error: function(error) {
		alert(error);
		}
	});
}


function changepass()
{
	if(str_token=='')
	{
		alert("您还未登陆，请先登陆！" );
		//$('#pwrequired').fadeOut();
		location.hash = '#sys_login';
		return;
	}
	var t_pass = document.getElementById("te_input_pasw").value;
	var t_pass2 = document.getElementById("te_input_pasw2").value;
	if(t_pass!=t_pass2)
	{
		alert("两次输入密码不匹配，请重新输入！");
		return;
	}
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/sys?auth="+str_token,
	  data: '{"method":"user.setpasswd","params":["root","'+t_pass+'"]}',
	  success: function() {
		alert("密码修改成功，下次登陆起效！")
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

function getwifiname()
{
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async:false,
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"foreach","params":["wireless","wifi-iface"]}',
	  success: function(json) {
		str_wifiname=json.result[0][".name"];
	  },
	  error: function(error) {
		alert(error);
		}
	});
}
function getthePara_rao(net,name,lable2)
{
	var label = document.getElementById(lable2);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	if(str_wifiname=="")
	{
		getwifiname();
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["'+net+'","'+str_wifiname+'","'+name+'"]}',
	  success: function(json) {
		label.innerHTML=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}
function get_pass_type(net,name,lable2)
{
	var label = document.getElementById(lable2);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	if(str_wifiname=="")
	{
		getwifiname();
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["'+net+'","'+str_wifiname+'","'+name+'"]}',
	  success: function(json) {
		  if(json.result=="none"||json.result=="NONE")
		  {
			  label.innerHTML="不加密";
		  }
		  else
		  {
			  label.innerHTML="加密（WPA2PSK）";
		  }
		
	  },
	  error: function(error) {
		alert('{"method":"get","params":["'+net+'","'+str_wifiname+'","'+name+'"]}');
		}
	});
}

function getthePara_rao2(net,name,lable2)
{
	var label = document.getElementById(lable2);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	if(str_wifiname=="")
	{
		getwifiname();
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["'+net+'","'+str_wifiname+'","'+name+'"]}',
	  success: function(json) {
		label.value=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}
function gettoken()
{
	var u_name='root';
	var u_passw=document.getElementById("pswd_login").value;
	//var label = document.getElementById("te_tocken");
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/auth",
	  data: '{"method":"login","params":["'+u_name+'","'+u_passw+'"]}',
	  success: function(json) {
		
		if(json.result!=null)
		{
			//label.innerHTML=json.result;
			str_token=json.result;
			str_username=u_name;
			str_userpass=u_passw;
			dosomething();
			setCookie("_token",json.result);
			location.hash = '#wifilist';
			
		}
		else
		{
			alert("登陆失败，重新登陆！");
		}
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

function getwan()
{
	var label = document.getElementById("te_wan_type");
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","proto"]}',
	  success: function(json) {
		label.innerHTML=json.result;
		str_wanType=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

function getwanip()
{
	var label = document.getElementById("ip");
	if(str_token=="")
	{
		//alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","ipaddr"]}',
	  success: function(json) {
		label.value=json.result;
		str_wanIp=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

function getwannetmask()
{
	var label = document.getElementById("mask");
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","netmask"]}',
	  success: function(json) {
		label.value=json.result;
		str_wanNetmask=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}	
function w_change_wifi()
{
	if(str_token=='')
	{
		alert("您还未登陆，请先登陆！" );
		$('#pwrequired').fadeOut();
		location.hash = '#sys_login';
		return;
	}
	var label = document.getElementById("ip_pswtype_select").innerHTML;
	var label_channel = document.getElementById("channel").innerHTML;
	if(label=="不加密")
	{
		setthePara_rao_value('wireless','encryption','none');
		
	}
	else
	{
		setthePara_rao_value('wireless','encryption','psk2');
		//setthePara_rao('wireless','encryption','te_ra0_iface2');
		setthePara_rao('wireless','ssid','ssid');
		setthePara_rao('wireless','key','wifi-pw');
	}
	setthePara_value('wireless','ra0','channel',label_channel);
	//setTimeout("commit(net)", 5000 );
	commit("wireless");
	
}
function change_wan()
{
	if(str_token=='')
	{
		alert("您还未登陆，请先登陆！" );
		$('#pwrequired').fadeOut();
		location.hash = '#sys_login';
		return;
	}
	var label = document.getElementById("ip_type_select").innerHTML;
	if(label=="动态获取")
	{
		setthePara_value('network','wan','proto','dhcp');
		setthePara_value('network','wan','dns','');
		setthePara_value('network','wan','ipaddr','');
		setthePara_value('network','wan','netmask','');
		setthePara_value('network','wan','gateway','');
		setthePara_value('network','wan','username','');
		setthePara_value('network','wan','password','');
	}
	else if(label=="固定IP")
	{
		setthePara_value('network','wan','proto','static');
		//setthePara('network','wan','ipaddr','ip');
		setthePara('network','wan','dns','edit_DNS');
		setthePara('network','wan','ipaddr','ip');
		setthePara('network','wan','netmask','mask');
		setthePara('network','wan','gateway','gate');
		setthePara_value('network','wan','username','');
		setthePara_value('network','wan','password','');
	}
	else
	{
		setthePara_value('network','wan','dns','');
		setthePara_value('network','wan','ipaddr','');
		setthePara_value('network','wan','netmask','');
		setthePara_value('network','wan','gateway','');
		setthePara_value('network','wan','proto','pppoe');
		setthePara('network','wan','username','pppoe-name');
		setthePara('network','wan','password','pppoe-pw');
	}
	//setTimeout("commit(net)", 5000 );
	commit("network");
}



function getwangateway()
{
	var label = document.getElementById("gate");
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","gateway"]}',
	  success: function(json) {
		label.value=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

function getwanUsername()
{
	var label = document.getElementById("pppoe-name");
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","gateway"]}',
	  success: function(json) {
		label.value=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}
function getthePara(net,type,name,lable)
{
	var label = document.getElementById(lable);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["'+net+'","'+type+'","'+name+'"]}',
	  success: function(json) {
		label.value=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

function getthePara_chanel(net,type,name,lable2)
{
	var label = document.getElementById(lable2);
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false,  
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["'+net+'","'+type+'","'+name+'"]}',
	  success: function(json) {
		label.innerHTML=json.result;
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

$(window).on('load', function () {
	getaps();
});	

function getaps() {
		  var ul=document.getElementById("ul_wifilist");
		  var ul2=document.getElementById("ul_mywifi");
		  ul2.innerHTML='';
		  ul.innerHTML='';
	     refleship(); 
        $.ajax({
          type: "GET", 
          url: "/cgi-bin/aps",
          dataType: "json",
          contentType: "application/json; charset=utf-8",
          success: function(json) {
              //$('.ul_wifilist').empty();
			  wifi_json=json;
			  
              for (var key in json) {
                  var cur_ap = json[key];
                  var chanel = cur_ap.chanel;
				  var security=cur_ap.security;
				  if (security!="none" && security!="NONE")
				  {
					  security="加密";
				  }
				  else
				  {
					  security="未加密";
				  }
				  var signal=cur_ap.signal;
				  var store = cur_ap.store;
				  var signal_class="q1";
				  if(signal>75)
				  {
					  signal_class="q4";
				  } 
				  else if(signal>50)
				  {
					  signal_class="q3";
				  } 
				  else if(signal>25)
				  {
					  signal_class="q2";
				  } 
				  var li= document.createElement("li"); 
				  li.className=signal_class;
				  if(store=="yes")
				  {
					li.innerHTML='<a onClick="inputthepassw(\''+key+'\')"><p class="wifititle">'+key+'(已保存)</p><p class="wificontent">'+security+'</p></a>';
					
				  }
				  else
				  {
					li.innerHTML='<a onClick="inputthepassw(\''+key+'\')"><p class="wifititle">'+key+'</p><p class="wificontent">'+security+'</p></a>';
				  }
				  ul.appendChild(li);
              }
              
          },
          error: function(error) {
            //alert("调用出错" + error.responseText);
          }
        });
		
}

function getstore (){
		  var ul2=document.getElementById("ul_mywifi");
		  ul2.innerHTML='';
			$.ajax({
          type: "GET", 
          url: "/cgi-bin/getstoressid",
          dataType: "json",
          contentType: "application/json; charset=utf-8",
          success: function(json) {
			  var tbl = document.getElementById("tbwifi");
			  for (var key in json)
			  {
				if(json[key]=="1")
				{
					var li= document.createElement("li");
					li.innerHTML='<p class="wifititle">'+key+'</p><a class="remove" onClick="delete_thiswifi(\''+key+'\')"></a>';
					ul2.appendChild(li);
				}
			  }
          },
          error: function(error) {
            //alert("调用出错" + error.responseText);
          }
        });
}

function setautoadd(){
	var checkbox=document.getElementById("autoadded")
	if (checkbox.checked)
	{
		setthePara_value('apcli0','system','enable','1');
	}
	else
	{
		setthePara_value('apcli0','system','enable','0');
	}

	$.ajax({
	  type: 'POST',
	  async : false, 
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"commit","params":["apcli0"]}',
	  success: function(json) {

	  },
	  error: function(error) {
		alert(error);
		}
	});
	
}

function getautoadd(){
	
	var checked = document.getElementById('autoadded');
	if(str_token=="")
	{
		alert("还未登陆，请登录！")
		return;
	}
	$.ajax({
	  type: 'POST',
	  async : false,  
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["apcli0","system","enable"]}',
	  success: function(json) {
		if(json.result== '1'){
			checked.checked="checked";
		}
		else
		{
			checked.checked="";
		}
	  },
	  error: function(error) {
		alert(error);
		}
	});
}

	  
function inputthepassw(name)
{
		choos_wifiname=name;
		var wifi_name=name;
		var security=wifi_json[wifi_name].security;
		if (security=="none" || security=="NONE")
		{
		$("#wireless_title").html("<b>是否加入"+name+"...</b>"); 
		document.getElementById('passwd_input').style.display='none';
		}
		else
		{
		$("#wireless_title").html("<b>加入"+name+"需要输入密码...</b>"); 
		document.getElementById('passwd_input').style.display='';
		}
		document.getElementById("password").value="";

		$('#pwrequired').fadeIn();
}
	  
	  
function setwifi() {
	if(str_token=="")
	{
		alert("您还未登陆，请先登陆！" );
		$('#pwrequired').fadeOut();
		location.hash = '#sys_login';
		return;
	}
	if(pwsavestate==0)
	{
		$.ajax({
		type: "GET", 
		url: "/cgi-bin/setwifi?ssid="+escape(choos_wifiname)+"&password="+escape($('#password').val()),
		dataType: "json",
		contentType: "application/json; charset=utf-8",
		success: function(json) {
		if (json.result == "error") {
			}
		else
			{
				location.reload(); 
			}			  
		},
			error: function(error) {
				//alert("调用出错" + error.responseText);
			}
		});
	}
		else
		{
			$.ajax({
			  type: "GET", 
			  url: "/cgi-bin/setstorewifi?ssid="+escape($('#ssid').val()),
			  dataType: "json",
			  contentType: "application/json; charset=utf-8",
			  success: function(json) {
				  if (json.result == "error") {
					
				  }
				  else
				  {
					 location.reload(); 
				  }			  
			  },
			  error: function(error) {
			    //alert("调用出错" + error.responseText);
			  }
			});			
		}
		$('#pwrequired').fadeOut();
		//alert("/cgi-bin/setwifi?test=sd dfsa&ssid="+document.getElementById("ssid").value+"&password="+$('#password').val());
		return false;
}
	  
	  
function refleship()
	{
		$.ajax({
          type: "GET", 
		  async:true,
          url: "/cgi-bin/checkwifi",
          dataType: "json",
          contentType: "application/json; charset=utf-8",
          success: function(json) {
              if (json.result=="success") {
                $("#status").text("已连接");
				$("#state_2").text("已连接");
				
				strs=json.ip.split(":");
				var ap=strs[1].split(" ");
				$("#status_ip3").text("IP:" + strs[2]);
				$("#status_ap").text(ap[0]);
				//$("#status_ap").innerHTML=ap[0]+$("#status_ap").innerHTML;
				$("#state_ap2").text(ap[0]);
				$("#state_ip2").text("IP:" + strs[2]);
				
				
              }
              else {
                $('#status').text("未连接外部WiFi");
				$('#state_2').text("未连接外部WiFi")
              }
          },
          error: function(error) {
            //alert("调用出错" + error.responseText);
          }
        });
	}
	
function checktocken(str_URL)
{
	if(str_token=="")
	{
		alert("您还未登陆，请先登陆！" );
		$('#pwrequired').fadeOut();
		location.hash = '#sys_login';
		return ;
	}
	else
	{
		location.hash =str_URL;
	}
	
	if(str_URL == "#wifilist"){
		getaps();
	}
	
	if(str_URL == "#mywifi"){
		getstore();
		refleship();
		getautoadd();
	}
	if(str_URL == "#wire-setting"){
	var label = document.getElementById("ip_type_select");
	$.ajax({
	  type: 'POST',
	  async:false,
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","proto"]}',
	  success: function(json) {
		if(json.result=="dhcp")
		{
			label.innerHTML="动态获取";
		} 
		else if(json.result=="static")
		{
			label.innerHTML="固定IP";
		}
		else
		{
			label.innerHTML="PPPOE";
		}
		tocken_live=true;
	  },
	  error: function(error) {
		}
	});
	$.ajax({
	  type: 'POST',
	  async:false,
	  url: "cgi-bin/luci/rpc/uci?auth="+str_token,
	  data: '{"method":"get","params":["network","wan","ifname"]}',
	  success: function(json) {
		if(json.result=="apcli0")
		{
			document.getElementById("setting_type").innerHTML="无线上网设置";
			document.getElementById("change-internet").value="切换到有线上网";
		} 
		else
		{
			document.getElementById("setting_type").innerHTML="有线上网设置";
			document.getElementById("change-internet").value="切换到无线上网";
		}
		tocken_live=true;
	  },
	  error: function(error) {
		}
	});

	}
}
	
	

function delete_thiswifi(wifi_id){
	$.ajax({
		type: "GET", 
		url: "/cgi-bin/delapcli?ssid="+escape(wifi_id),
		dataType: "json",
		contentType: "application/json; charset=utf-8",
			success: function(json) {
				if (json.result == "error") {
				}
				else
				{
					$(this).parent('li').fadeOut(500);
				}			  
			},
			error: function(error) {
				//alert("调用出错" + error.responseText);
				}
			});
			
	getstore();
}
	  