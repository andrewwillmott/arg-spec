Provides an example of ArgSpec usage

### Usage:
**test** [*options*] name:_string_ [dst:_string_]
<dl><dd>    Specify name and optionally destination for display    </dd></dl>

### Options:
**-v** 
<dl><dd>    Set verbose mode    </dd></dl>

**-size** _int_
<dl><dd>    Set image/window size    </dd></dl>

**-gamma** gamma:_double_
<dl><dd>    set gamma correction [default: 1.0]    </dd></dl>

**-cats** _bool_
<dl><dd>    Whether cats are enabled (default: false)    </dd></dl>

**-latlong** latitude:_float_ longitude:_float_
<dl><dd>    Set latitude and longitude    </dd></dl>

**-day** day:_int_
<dl><dd>    Set julian day [1..365]    </dd></dl>

**-counts** count1:_int_ ...
<dl><dd>    Specify counts using repeated last argument    </dd></dl>

**-countArray** counts:_int[]_
<dl><dd>    Specify counts as explicit array    </dd></dl>

**-names** name1:_string_ ...
<dl><dd>    List names    </dd></dl>

**-colour** _colourType_
<dl><dd>    Set colour    </dd></dl>

**-v2** _vec2_
<dl><dd>    Set v2    </dd></dl>

**-v3** _vec3_
<dl><dd>    Set v3    </dd></dl>

**-v4** _vec4_
<dl><dd>    Set v4    </dd></dl>

**-scale** _float_ [_float_ _float_]
<dl><dd>    Set uniform or xyz scale    </dd></dl>

**-h** [_helpType_]
<dl><dd>    Show full help, or help of the given type    </dd></dl>


**colourType:**

- red
- green
- blue
- black

**helpType:**

- brief
- full
- html
- md

