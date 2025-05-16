#include "mtypeinfo.h"

#include "widgets.h"
#include "TextEditor.h"
#include "imGuiFileDialog.h"
#include <mutex>

#include "../SDK/sdk.h"
#include "resources.h"

TextEditor structEditor;
TextEditor classEditor;
TextEditor enumEditor;

void typeinfo::draw()
{
    uintptr_t pFirstTypeInfo = OFFSET_FIRSTTYPEINFO;

    ImGui::Text("TypeInfo at 0x%X", pFirstTypeInfo);

    fb::TypeInfo** ppTypesList = reinterpret_cast<fb::TypeInfo**>(pFirstTypeInfo);
    fb::TypeInfo* pFirstType = *ppTypesList;
    fb::TypeInfo* pCurrentType = pFirstType;

    static std::vector<std::string> declarationsNames;
    static std::vector<std::string> structs;
    static std::vector<std::string> classes;
    static std::vector<std::string> enums;
    static std::vector<std::string> structsNames;
    static std::vector<std::string> classesNames;
    static std::vector<std::string> enumsNames;

    static FBBackend::SDK_GENERATOR sdkGEn{ };
    static MinimalResourceEditor resourceEditor{ };

    static uint64_t count = 0;

    if (ImGui::Button("Generate dump"))
    {
        count = 0;
        sdkGEn.Destroy();
        resourceEditor.Destroy();

        declarationsNames.clear();
        structs.clear();
        classes.clear();
        enums.clear();

        structsNames.clear();
        classesNames.clear();
        enumsNames.clear();

        do
        {
            sdkGEn.Register(pCurrentType);
            count++;
        } while ((pCurrentType = pCurrentType->m_Next) != NULL);

        sdkGEn.Analyze();

        resourceEditor.Initialize(&sdkGEn);

        sdkGEn.GenerateDeclarations(declarationsNames);
        sdkGEn.GenerateStructs(structs, structsNames);
        sdkGEn.GenerateClasses(classes, classesNames);
        sdkGEn.GenerateEnums(enums, enumsNames);
    }

    if (ImGui::CollapsingHeader("Typeinfo", ImGuiTreeNodeFlags_None))
    {
        static std::string declarationsPath = "declarations.h";
        static std::string structsPath = "structs.h";
        static std::string classesPath = "classes.h";
        static std::string enumsPath = "enums.h";

        static std::once_flag oF;
        std::call_once(oF, []()
            {
                structEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
                classEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
                enumEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
            });

        
        static IGFD::FileDialogConfig config;
        config.path = ".";

        if (ImGui::Button("Select Declarations Path"))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".h", config);
        }
        if (ImGuiFileDialog::Instance()->IsOpened() && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                declarationsPath = ImGuiFileDialog::Instance()->GetFilePathName();
            }

            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SameLine();
        ImGui::Text("%s", declarationsPath.c_str());

        if (ImGui::Button("Select Structs Path"))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".h", config);
        }

        if (ImGuiFileDialog::Instance()->IsOpened() && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                structsPath = ImGuiFileDialog::Instance()->GetFilePathName();
            }

            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SameLine();
        ImGui::Text("%s", structsPath.c_str());

        if (ImGui::Button("Select Classes Path"))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".h", config);
        }
        if (ImGuiFileDialog::Instance()->IsOpened() && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                classesPath = ImGuiFileDialog::Instance()->GetFilePathName();
            }

            ImGuiFileDialog::Instance()->Close();
        }
        ImGui::SameLine();
        ImGui::Text("%s", classesPath.c_str());

        if (ImGui::Button("Select Enums Path"))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".h", config);
        }
        if (ImGuiFileDialog::Instance()->IsOpened() && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                enumsPath = ImGuiFileDialog::Instance()->GetFilePathName();
            }

            ImGuiFileDialog::Instance()->Close();
        }
        ImGui::SameLine();
        ImGui::Text("%s", enumsPath.c_str());

        ImGui::Checkbox("Add comments about typeinfo", &sdkGEn.m_generateAboutTypeinfo);
        ImGui::Checkbox("Add static members", &sdkGEn.m_generateStaticMembers);

        ImGui::SameLine();
        ImGui::TextColored(count ? ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f } : ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, "Count %llu", count);

        if (count)
        {
            ImGui::Text("Typeinfos %llu", static_cast<unsigned long long>(sdkGEn.m_typeInfos->size()));
            ImGui::Text("ClassInfos %llu", static_cast<unsigned long long>(sdkGEn.m_classInfos->size()));
            ImGui::Text("EnumInfos %llu", static_cast<unsigned long long>(sdkGEn.m_enumInfos->size()));
            ImGui::Text("ValueInfos %llu", static_cast<unsigned long long>(sdkGEn.m_valueInfos->size()));
            ImGui::Text("SDK Declarations %llu", static_cast<unsigned long long>(declarationsNames.size()));
            ImGui::Text("SDK Classes %llu", static_cast<unsigned long long>(sdkGEn.m_sdkClasses->size()));
            ImGui::Text("SDK Enums %llu", static_cast<unsigned long long>(sdkGEn.m_sdkEnums->size()));
            ImGui::Text("SDK Structs %llu", static_cast<unsigned long long>(sdkGEn.m_sdkStructs->size()));


            if (ImGui::CollapsingHeader("Searcher", ImGuiTreeNodeFlags_None))
            {
                static int idxStruct = 0;
                static int idxClass = 0;
                static int idxEnum = 0;

                auto drawStructs = []()
                    {
                        if (ImGui::ComboWithFilter("Struct search", &idxStruct, structsNames))
                        {
                            structEditor.SetText(structs.at(idxStruct));
                        }

                        if (idxStruct >= 0)
                        {
                            structEditor.Render("##structs");
                        }
                    };

                auto drawClasses = []()
                    {
                        if (ImGui::ComboWithFilter("Class search", &idxClass, classesNames))
                        {
                            classEditor.SetText(classes.at(idxClass));
                        }

                        if (idxClass >= 0)
                        {
                            classEditor.Render("##classes");
                        }
                    };

                auto drawEnums = []()
                    {
                        if (ImGui::ComboWithFilter("Enum search", &idxEnum, enumsNames))
                        {
                            enumEditor.SetText(enums.at(idxEnum));
                        }

                        if (idxEnum >= 0)
                        {
                            enumEditor.Render("##enums");
                        }
                    };

                static std::array<TabRender, 3> allSDKTabs =
                {
                    TabRender{ "Structs##t", drawStructs },
                    TabRender{ "Classes##t", drawClasses },
                    TabRender{ "Enums##t", drawEnums },
                };

                ImGui::BeginChild("##childsearch");
                {
                    if (ImGui::BeginTabBar("sdktabs", ImGuiTabBarFlags_Reorderable))
                    {
                        for (const auto& el : allSDKTabs)
                        {
                            if (ImGui::BeginTabItem(el.m_name))
                            {
                                if (el.funcExist())
                                {
                                    el.m_func();
                                }
                                ImGui::EndTabItem();
                            }
                        }
                        ImGui::EndTabBar();
                    }
                }
                ImGui::EndChild();
            }

            if (ImGui::Button("Save dump"))
            {
                auto saveToFile = [](const std::string& path, const std::vector<std::string>& data)
                    {
                        std::ofstream file(path);
                        if (file.is_open())
                        {
                            for (const auto& line : data)
                            {
                                file << line << std::endl << std::endl;
                            }
                        }
                    };

                saveToFile(declarationsPath, declarationsNames);
                saveToFile(structsPath, structs);
                saveToFile(classesPath, classes);
                saveToFile(enumsPath, enums);
            }
        }
    }

    if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_None))
    {
        resourceEditor.Render();
    }
}