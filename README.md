Provides an example of ArgSpec usage

## Usage:
> test [*options*] name:_string_ [dst:_string_]
>> Specify name and optionally destination for display

## Options:
>   **-v** 
>>    Set verbose mode

>   **-size** _int_
>>    Set image/window size

>   **-gamma** gamma:_double_
>>    set gamma correction [default: 1.0]

>   **-cats** _bool_
>>    Whether cats are enabled (default: false)

>   **-latlong** latitude:_float_ longitude:_float_
>>    Set latitude and longitude

>   **-day** day:_int_
>>    Set julian day [1..365]

>   **-counts** count1:_int_ ...
>>    Specify counts using repeated last argument

>   **-countArray** counts:_int[]_
>>    Specify counts as explicit array

>   **-names** name1:_string_ ...
>>    List names

>   **-colour** _colourType_
>>    Set colour

>   **-v2** _vec2_
>>    Set v2

>   **-v3** _vec3_
>>    Set v3

>   **-v4** _vec4_
>>    Set v4

>   **-scale** _float_ [_float_ _float_]
>>    Set uniform or xyz scale

>   **-h** [_helpType_]
>>    Show full help, or help of the given type


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


