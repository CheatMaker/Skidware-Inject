#include "reach.h"
#include "../../../sdk/sdk.h"
#include "../../commonData.h"
#include "../../../util/logger.h"
#include "../../../java/java.h"
#include "../../../util/math/math.h"
#include "../../../../../ext/imgui/imgui.h"
#include "../../../menu/menu.h"

#include <chrono>

Reach::Reach() : AbstractModule("Reach", Category::COMBAT) {
	EventManager::getInstance().reg<EventUpdate>([this](auto&& PH1) { onUpdate(std::forward<decltype(PH1)>(PH1)); });
}

Reach* Reach::getInstance() {
	static auto* inst = new Reach();
	return inst;
}

void Reach::onDisable() {
}

void Reach::onEnable() {
}


std::chrono::steady_clock::time_point lastUpdate;
void Reach::onUpdate(const EventUpdate e)
{
	if (!this->getToggle()) return;
	if (!CommonData::getInstance()->SanityCheck()) return;

	std::chrono::steady_clock::time_point nanoTime = std::chrono::high_resolution_clock::now();

	// Calculate the elapsed time in nanoseconds
	float elapsed = (float) std::chrono::duration_cast<std::chrono::nanoseconds>(nanoTime - lastUpdate).count() / 1000000;
	if (elapsed < 20)
		return;
	else
		lastUpdate = nanoTime;

	CEntityPlayerSP* thePlayer = SDK::Minecraft->thePlayer;
	List playerList = CommonData::getInstance()->playerEntities;

	Vector2 playerAngles = thePlayer->GetAngles();
	Vector3 playerPos = thePlayer->GetPos();

	for (CEntityPlayer target : playerList.toVector<CEntityPlayer>())
	{
		if (!target.isValid() || target.isNULL()) continue;

		float distance = ReachDistance;
		if (Java::Env->IsSameObject(thePlayer->getInstance(), target.getInstance())) continue;

		BoundingBox targetBB = target.GetBB().GetNativeBoundingBox();

		float hypothenuseDistance = (float) sqrt(pow((playerPos - target.GetPos()).x, 2) + pow((playerPos - target.GetPos()).z, 2));

		if (distance > hypothenuseDistance)
			distance -= hypothenuseDistance;

		float difference = Math::wrapAngleTo180(playerAngles.x - Math::getAngles(playerPos, target.GetPos()).x);

		if (std::abs(difference) > 180.0f)
			continue;

		float cos = std::cos(Math::degToRadiants(Math::getAngles(playerPos, target.GetPos()).x + 90.0f));
		float sin = std::sin(Math::degToRadiants(Math::getAngles(playerPos, target.GetPos()).x + 90.0f));
		float cosPitch = std::cos(Math::degToRadiants(Math::getAngles(playerPos, target.GetPos()).y));
		float sinPitch = std::sin(Math::degToRadiants(Math::getAngles(playerPos, target.GetPos()).y));

		float x = target.GetPos().x - (cos * distance * cosPitch);
		float y = target.GetPos().y + (distance * sinPitch);
		float z = target.GetPos().z - (sin * distance * cosPitch);

		float entityWidth = 0.6f;
		float bbWidth = entityWidth / 2.0f;

		BoundingBox newBB{
			(double) x - bbWidth,
			(double) y,
			//targetBB.minY,
			(double) z - bbWidth,

			(double) x + bbWidth,
			(double) y + 2,
			//targetBB.maxY,
			(double) z + bbWidth
		};

		target.SetBB(newBB);
	}

}

void Reach::RenderMenu()
{

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 0.5));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10);

	if (ImGui::BeginChild("reach", ImVec2(450, 75))) {

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		Menu::DoToggleButtonStuff(230044, "Toggle Reach", this);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
		ImGui::Separator();
		ImGui::SliderFloat("Reach Distance", &this->ReachDistance, 0, 4);

		ImGui::EndChild();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

}
