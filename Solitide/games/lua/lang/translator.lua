-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

return function(category, id, context)
    if category == "rule" then
        return Locale.rule[id](context);
    end

    return Locale[category][id];
end
