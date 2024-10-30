<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta http-equiv="Content-Style-Type" content="text/css">
  <title></title>
  <meta name="Generator" content="Cocoa HTML Writer">
  <meta name="CocoaVersion" content="2487.6">
  <style type="text/css">
    p.p1 {margin: 0.0px 0.0px 0.0px 0.0px; font: 14.0px Menlo; color: #000000}
    p.p2 {margin: 0.0px 0.0px 0.0px 0.0px; font: 14.0px Menlo; color: #000000; min-height: 16.0px}
    span.s1 {font-variant-ligatures: no-common-ligatures}
  </style>
</head>
<body>
<p class="p1"><span class="s1">#include &lt;stdio.h&gt;</span></p>
<p class="p1"><span class="s1">#include &lt;unistd.h&gt;</span></p>
<p class="p1"><span class="s1">#include &lt;sys/types.h&gt;</span></p>
<p class="p1"><span class="s1">#include &lt;errno.h&gt;</span></p>
<p class="p1"><span class="s1">#include &lt;stdlib.h&gt;</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p1"><span class="s1">void print_uids() {</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>printf("Real UID: %d\n", getuid());</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>printf("Effective UID: %d\n", geteuid());</span></p>
<p class="p1"><span class="s1">}</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p1"><span class="s1">int main() {</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>printf("Before setuid:\n");</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>print_uids();</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>FILE* file = fopen("myfile.txt", "r");</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>if (file == NULL) {</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">        </span>perror("Error opening file");</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>}</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>else {</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">        </span>fclose(file);</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>}</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p2"><span class="s1"><span class="Apple-converted-space">   </span></span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>if (setuid(getuid()) != 0) {</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">        </span>perror("setuid failed");</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">        </span>exit(1);</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>}</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p2"><span class="s1"><span class="Apple-converted-space">   </span></span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>printf("After setuid:\n");</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>print_uids();</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>file = fopen("myfile.txt", "r");</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>if (file == NULL) {</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">        </span>perror("Error opening file after setuid");</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>}</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>else {</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">        </span>fclose(file);</span></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>}</span></p>
<p class="p2"><span class="s1"></span><br></p>
<p class="p1"><span class="s1"><span class="Apple-converted-space">    </span>return 0;</span></p>
<p class="p1"><span class="s1">}</span></p>
</body>
</html>
