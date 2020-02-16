local explo
function love.load()
	explo = love.audio.newSource("assets/explosion_basic.ogg", "static")
	explo:play()
end

function love.draw()
	love.graphics.print("Psychic Saboteurs", 400, 300)
end