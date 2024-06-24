-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

return function(category, id, context)
    if category == "rule" then
        return Locale.rule[id](context);
    elseif category == "family" then
        if type(id) ~= 'table' then return Locale.family[id]; end
        local str = ""
        for _, fam in ipairs(id) do
            if str ~= "" then str = str .. "/" end
            str = str .. Locale.family[fam]
        end
        return str
    end

    return Locale[category][id];
end
