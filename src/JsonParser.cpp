// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "JsonParser.hpp"
#include "Vector.hpp"
#include "stack.hpp"
#include <chrono>
#include <cstdint>
#include <ostream>
#include <pthread.h>
#include <cassert>

namespace GLVM::Core
{    

    void CJsonParser::ReadFile(const char* _filePath) {
        const char* _pJsonFilePath = _filePath;
        std::ifstream jsonFileInputStream;
        std::stringstream jsonFileOutputStream;
        
        jsonFileInputStream.open(_pJsonFilePath);
        if(jsonFileInputStream.good()) {

            jsonFileOutputStream << jsonFileInputStream.rdbuf();
            jsonFileInputStream.close();
            sJsonFileData_ = jsonFileOutputStream.str();
        } else {
            std::cout << "Error of reading json file" << std::endl;
            return;
        }

        pJsonFileData_ = sJsonFileData_.c_str();
    }

    void CJsonParser::Parse() {
		currentChar_ = pJsonFileData_[globalFileCounter_];
		
		while (currentChar_ != '\0') {
			currentChar_ = pJsonFileData_[globalFileCounter_];

			if (currentChar_ == '"' && keyFlag) {
				lastKey_ = StringParse();
				while (currentChar_ == ' ' || currentChar_ == ':') {
				++globalFileCounter_;
				currentChar_ = pJsonFileData_[globalFileCounter_];
				}
			}
			
			if (currentChar_ == '"') {
				bufferString_ = StringParse();

				if (keyFlag) {
					JsonValue jsonString(bufferString_);
					(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonString;
				} else {
 					JsonValue jsonString(bufferString_);
					stackOfJsonValues_.GetHead()->value.array->Push(jsonString);
				}
			} else if ((currentChar_ >= '0' && currentChar_ <= '9') ||
					   currentChar_ == '+' || currentChar_ == '-') {
				bufferString_ = NumberAsStringParse();
				core::vector<char> vector = StringToVectorOfChars(bufferString_);
				double fNumber = 0.0f;
				int iNumber = 0;
				if (IsContainChar(bufferString_, '.')) {
					fNumber = ParseFloating(vector);

					if (keyFlag) {
						JsonValue jsonFloat(fNumber);
						(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonFloat;
					} else {
						JsonValue jsonFloat(fNumber);
						stackOfJsonValues_.GetHead()->value.array->Push(jsonFloat);
					}
				} else {
 					iNumber = ParseInteger(vector);

					if (keyFlag) {
						JsonValue jsonInt(iNumber);
						(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonInt;
					} else {
						JsonValue jsonInt(iNumber);
						stackOfJsonValues_.GetHead()->value.array->Push(jsonInt);
					}
				}

			} else if (currentChar_ == 't' ||
				       currentChar_ == 'f' ||
				       currentChar_ == 'n') {
				std::string boolOrNullString = BoolOrNullParse();

				if (boolOrNullString == "true") {
					if (keyFlag) {
						JsonValue jsonTrue(true);
						(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonTrue;
					} else {
						JsonValue jsonTrue(true);
						stackOfJsonValues_.GetHead()->value.array->Push(jsonTrue);
					}
				} else if (boolOrNullString == "false") {
					if (keyFlag) {
						JsonValue jsonFalse(false);
						(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonFalse;
					} else {
						JsonValue jsonFalse(false);
						stackOfJsonValues_.GetHead()->value.array->Push(jsonFalse);
					}
				} else if (boolOrNullString == "null") {
					if (keyFlag) {
						JsonValue jsonNull;
						jsonNull.type = JSON_NULL;
						jsonNull.value.null = NULL;
						(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonNull;
					} else {
						JsonValue jsonNull;
						jsonNull.type = JSON_NULL;
						jsonNull.value.null = NULL;
						stackOfJsonValues_.GetHead()->value.array->Push(jsonNull);
					}
				}
			} else if (currentChar_ == '{') {
				if (stackOfJsonValues_.GetSize() == 0) {
					root_ = new JsonValue;
					*root_ = CreateJsonHashMap();
					stackOfJsonValues_.Push(root_);
				} else if (keyFlag) {
					JsonValue jsonObject = CreateJsonHashMap();
					(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonObject;
					stackOfJsonValues_.Push(&(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()]);
				} else if (!keyFlag) {
					JsonValue jsonObject = CreateJsonHashMap();
					stackOfJsonValues_.GetHead()->value.array->Push(jsonObject);
					stackOfJsonValues_.Push(&stackOfJsonValues_.GetHead()->value.array->GetHead());
				}

				keyFlag = true;
			} else if (currentChar_ == '[') {
				if (stackOfJsonValues_.GetSize() == 0) {
					root_ = new JsonValue;
					*root_ = CreateJsonArray();
					stackOfJsonValues_.Push(root_);
				} else if (keyFlag) {
					JsonValue jsonArray = CreateJsonArray();
					(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()] = jsonArray;
					stackOfJsonValues_.Push(&(*stackOfJsonValues_.GetHead()->value.object)[lastKey_.c_str()]);
				} else if (!keyFlag) {
					JsonValue jsonArray = CreateJsonArray();
					stackOfJsonValues_.GetHead()->value.array->Push(jsonArray);
					stackOfJsonValues_.Push(&stackOfJsonValues_.GetHead()->value.array->GetHead());
				}

				keyFlag = false;
			} else if (currentChar_ == '}') {
				stackOfJsonValues_.Pop();
				if (stackOfJsonValues_.GetSize() && stackOfJsonValues_.GetHead()->type == JSON_OBJECT)
					keyFlag = true;
				else
					keyFlag = false;

			} else if (currentChar_ == ']') {
				stackOfJsonValues_.Pop();
				if (stackOfJsonValues_.GetSize() && stackOfJsonValues_.GetHead()->type == JSON_OBJECT)
					keyFlag = true;
				else
					keyFlag = false;

			} 
			
			++globalFileCounter_;
		}
	}

	JsonValue CJsonParser::CreateJsonHashMap() {
		JsonValue jsonObject;
		jsonObject.type = JSON_OBJECT;
		jsonObject.value.object = new HashMap<JsonValue>;
		return jsonObject;
	}

	JsonValue CJsonParser::CreateJsonArray() {
		JsonValue jsonArray;
		jsonArray.type = JSON_ARRAY;
		jsonArray.value.array = new core::vector<JsonValue>;
		return jsonArray;
	}
	
	std::string CJsonParser::BoolOrNullParse() {
		std::string boolOrNullString = "";
		while (1) {
			currentChar_ = pJsonFileData_[globalFileCounter_];
			if (currentChar_ >= 'a' && currentChar_ <= 'z') {
				boolOrNullString.push_back(currentChar_);
				++globalFileCounter_;
			} else
				return boolOrNullString;
		}
	}
	
	bool CJsonParser::IsContainChar(std::string _string, char _char) {
		for (unsigned int i = 0; i < _string.size(); ++i) {
			if (_string[i] == _char)
				return true;
		}

		return false;
	}
			
	std::string CJsonParser::NumberAsStringParse() {
		std::string numberAsString = "";
		while (1) {
			currentChar_ = pJsonFileData_[globalFileCounter_];
			if ((currentChar_ >= '0' && currentChar_ <= '9') ||
				currentChar_ == '+' || currentChar_ == '-' ||
				currentChar_ == 'e') {
				numberAsString.push_back(currentChar_);
				++globalFileCounter_;
			} else if (currentChar_ == '.') {
				numberAsString.push_back(currentChar_);
				++globalFileCounter_;
			} else
				return numberAsString;
		}
	}
		
 	std::string CJsonParser::StringParse() {
		++globalFileCounter_;
		std::string localBuffer = "";
		while (1) {
			currentChar_ = pJsonFileData_[globalFileCounter_];
			if (currentChar_ == '"') {
				++globalFileCounter_;
				currentChar_ = pJsonFileData_[globalFileCounter_];
				return localBuffer;
			} else {
				localBuffer.push_back(currentChar_);
				++globalFileCounter_;
			}
		}
	}
	
	core::vector<char> CJsonParser::StringToVectorOfChars(std::string _string) {
		core::vector<char> vectorWithChars;
		for (unsigned int i = 0; i < _string.size(); ++i) {
			vectorWithChars.Push(_string[i]);
		}

		return vectorWithChars;
	}
	
	int CJsonParser::ParseInteger(core::vector<char> _word) {
		core::vector<int> baseContainer;

        for (unsigned int i = 0; i < _word.GetSize(); ++i)
            baseContainer.Push(_word[i] - 48);

        int iResult = 0;
        bool negateFlag = false;

        unsigned int baseContainerSize = baseContainer.GetSize();
        for (unsigned int i = 0; i < baseContainerSize; ++i) {
            if (baseContainer[i] == -3 && i == 0) {
				negateFlag = true;
                continue;
            } else if (baseContainer[i] == -5 && i == 0)
                continue;

            iResult += baseContainer[i] * std::pow(10, (baseContainerSize - 1) - i);
        }

		if (negateFlag)
			iResult *= -1;
		
        return iResult;
    }
    
    double CJsonParser::ParseFloating(core::vector<char> _word) {
		core::vector<int> baseContainer;

        for (unsigned int i = 0; i < _word.GetSize(); ++i)
            baseContainer.Push(_word[i] - 48);

        int integerPart = 0;
        double floatingPart = 0;
		int eNumber = 0;
		core::vector<int> integerPartContainer;
		core::vector<int> floatingPartContainer;
		core::vector<int> ePartContainer;
        bool dotFlag = false;
        bool negateFlag = false;
		bool eFlag = false;
		bool eSign = false;                    ///< false value equal "+" sign;
        unsigned int baseContainerSize = baseContainer.GetSize();

        if (baseContainer[0] == -3)
            negateFlag = true;

        for (unsigned int i = 0; i < baseContainerSize; ++i) {
            if (negateFlag && i == 0)
                continue;
            else if (baseContainer[i] == -5 && i == 0)
                continue;
            else if (baseContainer[i] == -2) {
                dotFlag = true;
                continue;
            } else if (baseContainer[i] == 53) {
				eFlag = true;
				continue;
			}

			if (eFlag) {
				if (baseContainer[i] == -5)
					continue;
				else if (baseContainer[i] == -3) {
					eSign = true;
					continue;
				}

				ePartContainer.Push(baseContainer[i]);
				continue;
			}
			
            if (baseContainer[i] >= 0 && baseContainer[i] <= 9) {
                if (dotFlag)
                    floatingPartContainer.Push(baseContainer[i]);
                else
                    integerPartContainer.Push(baseContainer[i]);
            } else {
                std::cout << "Element is not a number" << std::endl;
                return NAN;
            }
        }

        unsigned int ePartContainerSize = ePartContainer.GetSize();
        for (unsigned int i = 0; i < ePartContainerSize; ++i)
            eNumber += ePartContainer[i] * std::pow(10, (ePartContainerSize - 1) - i);
		
        unsigned int integerPartContainerSize = integerPartContainer.GetSize();
        for (unsigned int i = 0; i < integerPartContainerSize; ++i)
            integerPart += integerPartContainer[i] * std::pow(10, (integerPartContainerSize - 1) - i);

        unsigned int floatingPartContainerSize = floatingPartContainer.GetSize();
        for (unsigned int i = 0; i < floatingPartContainerSize; ++i)
            floatingPart += floatingPartContainer[i] / std::pow(10, i + 1);

        double result = 0;
        result = (double)(integerPart + floatingPart);

		if (eFlag) {
			if (eSign)
				result /= std::pow(10, eNumber);
			else
				result *= std::pow(10, eNumber);
		}
		
        if (negateFlag)
            result *= -1.0f;

        return result;
    }

	void CJsonParser::SearchInJsonArray(core::vector<JsonValue>* arrayValue, const char* key_,
										core::vector<JsonValue>& resultVector) const {
		for ( unsigned int i = 0; i < arrayValue->GetSize(); ++i ) {
			if ( (*arrayValue)[i].type == JSON_OBJECT )
				SearchInJsonObject((*arrayValue)[i].value.object, key_, resultVector);

			if ( (*arrayValue)[i].type == JSON_ARRAY )
				SearchInJsonArray((*arrayValue)[i].value.array, key_, resultVector);
		}
	}
	
	void CJsonParser::SearchInJsonObject(HashMap<JsonValue>* mapValue, const char* key_,
										 core::vector<JsonValue>& resultVector) const {
		for ( unsigned int i = 0; i < mapValue->GetCapacity(); ++i ) {
			if ( mapValue->hashMap_[i] != nullptr ) {
				Node<JsonValue>* current = mapValue->hashMap_[i];
				while ( current != nullptr ) {
					std::string searchKey = key_;
					std::string currentKey = current->key_;
					if ( currentKey == searchKey ) {
						resultVector.Push(current->value_);
					}

					if ( current->value_.type == JSON_OBJECT )
						SearchInJsonObject(current->value_.value.object, key_, resultVector);

					if ( current->value_.type == JSON_ARRAY )
						SearchInJsonArray(current->value_.value.array, key_, resultVector);

					current = current->next_;
				}
			}
		}
	}		
	
	core::vector<JsonValue> CJsonParser::Search(const char* key_) const {
		core::vector<JsonValue> resultVector;
		SearchInJsonObject(root_->value.object, key_, resultVector);
		return resultVector;
	}

	void CJsonParser::LoadGLTF(const char* pathsGLTF_,
							   std::vector<float>& aVertexes_,
							   std::vector<uint32_t>& aIndices_,
							   core::vector<core::vector<mat4>>& jointMatricesPerMesh,
							   core::vector<float>& frames,
							   bool& noAnimations) {
		ReadFile(pathsGLTF_);
		Parse();
		
		Core::JsonValue* gltf = GetRoot();
		std::string binary_path = *(*gltf)["buffers"][0]["uri"].value.string;
		int full_byte_size = (*gltf)["buffers"][0]["byteLength"].value.iNumber;;
		std::ifstream in_stream;
		in_stream.open("../gltf/" + binary_path, std::ios::binary);
		char* buffer = new char[full_byte_size];
		in_stream.read(buffer, full_byte_size);
		in_stream.close();

		int indices_index = (*gltf)["meshes"][0]["primitives"][0]["indices"].value.iNumber;
		int indices_buffer_view_index = (*gltf)["accessors"][indices_index]["bufferView"].value.iNumber;
		int indices_byte_length = (*gltf)["bufferViews"][indices_buffer_view_index]["byteLength"].value.iNumber;
		int indices_byte_offset = (*gltf)["bufferViews"][indices_buffer_view_index]["byteOffset"].value.iNumber;

		core::vector<unsigned int> indices;
		for ( int i = indices_byte_offset; i < indices_byte_offset + indices_byte_length; i += 2 )
			indices.Push(reinterpret_cast<unsigned short &>(buffer[i]));

		int vertices_position_index = (*gltf)["meshes"][0]["primitives"][0]["attributes"]["POSITION"].value.iNumber;
		int vertices_buffer_view_index = (*gltf)["accessors"][vertices_position_index]["bufferView"].value.iNumber;
		int vertices_byte_length = (*gltf)["bufferViews"][vertices_buffer_view_index]["byteLength"].value.iNumber;
		int vertices_byte_offset = (*gltf)["bufferViews"][vertices_buffer_view_index]["byteOffset"].value.iNumber;

		core::vector<float> vertices_position;
		for ( int i = vertices_byte_offset; i < vertices_byte_offset + vertices_byte_length; i += 4 )
			vertices_position.Push(reinterpret_cast<float &>(buffer[i]));

		int texture_coordinates_index = (*gltf)["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"].value.iNumber;
		int texture_buffer_view_index = (*gltf)["accessors"][texture_coordinates_index]["bufferView"].value.iNumber;
		int texture_byte_length = (*gltf)["bufferViews"][texture_buffer_view_index]["byteLength"].value.iNumber;
		int texture_byte_offset = (*gltf)["bufferViews"][texture_buffer_view_index]["byteOffset"].value.iNumber;

		core::vector<float> texture_coordinates;
		for ( int i = texture_byte_offset; i < texture_byte_offset + texture_byte_length; i += 4 )
			texture_coordinates.Push(reinterpret_cast<float &>(buffer[i]));

		int normals_index = (*gltf)["meshes"][0]["primitives"][0]["attributes"]["NORMAL"].value.iNumber;
		int normals_buffer_view_index = (*gltf)["accessors"][normals_index]["bufferView"].value.iNumber;
		int normals_byte_length = (*gltf)["bufferViews"][normals_buffer_view_index]["byteLength"].value.iNumber;
		int normals_byte_offset = (*gltf)["bufferViews"][normals_buffer_view_index]["byteOffset"].value.iNumber;

		core::vector<float> normals;
		for ( int i = normals_byte_offset; i < normals_byte_offset + normals_byte_length; i += 4 )
			normals.Push(reinterpret_cast<float &>(buffer[i]));

		core::vector<Core::JsonValue> skins = Search("skins");
		Core::JsonValue joints;
		core::vector<mat4> globalTransformJointNode;
		core::vector<mat4> inverseBindMatrixSet;
		core::vector<core::vector<mat4>> jointMatrices;
		core::vector<float> weightsContainer;
		core::vector<int> jointsIndices;
		core::vector<core::vector<int>> children;
			
		if ( skins.GetSize() > 0 ) {
			noAnimations = false;
			joints = (*gltf)["skins"][0]["joints"];

			Core::JsonValue nodes = (*gltf)["nodes"];
			for ( unsigned int i = 0; i < joints.value.array->GetSize(); ++i ) {
				unsigned int index = (*joints.value.array)[i].value.iNumber;
				Core::JsonValue node = nodes[index];
				Quaternion rotationQuaternion;
				mat4 rotation(1.0f);
				mat4 scale(1.0f);
				mat4 translation(1.0f);

				if ( node.value.object->Contain("rotation") ) {
					Core::JsonValue array = (*node.value.object)["rotation"];
					for ( unsigned int i = 0; i < array.value.array->GetSize(); ++i ) {
						switch(i) {
						case 0:
							if ( array[i].isInterger() )
								rotationQuaternion.x = array[i].value.iNumber;
							else if ( array[i].isFloat() )
								rotationQuaternion.x = array[i].value.fNumber;
						case 1:
							if ( array[i].isInterger() )
								rotationQuaternion.y = array[i].value.iNumber;
							else if ( array[i].isFloat() )
								rotationQuaternion.y = array[i].value.fNumber;
						case 2:
							if ( array[i].isInterger() )
								rotationQuaternion.z = array[i].value.iNumber;
							else if ( array[i].isFloat() )
								rotationQuaternion.z = array[i].value.fNumber;
						case 3:
							if ( array[i].isInterger() )
								rotationQuaternion.w = array[i].value.iNumber;
							else if ( array[i].isFloat() )
								rotationQuaternion.w = array[i].value.fNumber;
						}
					}

					rotation = rotateQuaternion<float, 4>(rotationQuaternion);
				}

				core::vector<int> local_children;
				if ( node.value.object->Contain("children") ) {
					Core::JsonValue array = (*node.value.object)["children"];
					for ( unsigned int i = 0; i < array.value.array->GetSize(); ++i ) {
						local_children.Push(array[i].value.iNumber);
					}

					children.Push(local_children);
				}
				else {
					core::vector<int> emptyChildren;
//					emptyChildren.Push(-1);
					children.Push(emptyChildren);
				}
				
				if ( node.value.object->Contain("scale") ) {
					Core::JsonValue array = (*node.value.object)["scale"];
					for ( unsigned int i = 0; i < array.value.array->GetSize(); ++i ) {
						if ( array[i].isInterger() )
							scale[i][i] = array[i].value.iNumber;
						else if ( array[i].isFloat() )
							scale[i][i] = array[i].value.fNumber;
					}
				}

				if ( node.value.object->Contain("translation") ) {
					Core::JsonValue array = (*node.value.object)["translation"];
					for ( unsigned int i = 0; i < array.value.array->GetSize(); ++i ) {
						if ( array[i].isInterger() )
							translation[3][i] = array[i].value.iNumber;
						else if ( array[i].isFloat() )
							translation[3][i] = array[i].value.fNumber;
					}


				}
				
				mat4 model = translation * scale * rotation;
				globalTransformJointNode.Push(model);
			}

			unsigned int inverseBindMatricesIndex = (*gltf)["skins"][0]["inverseBindMatrices"].value.iNumber;
			unsigned int bufferView = (*gltf)["accessors"][inverseBindMatricesIndex]["bufferView"].value.iNumber;
			unsigned int byteLengthInverseBindMatrices = (*gltf)["bufferViews"][bufferView]["byteLength"].value.iNumber;
			unsigned int byteOffsetInverseBindMatrices = (*gltf)["bufferViews"][bufferView]["byteOffset"].value.iNumber;

			core::vector<float> inverseBindMatricesData;
			for ( unsigned int i = byteOffsetInverseBindMatrices; i < byteOffsetInverseBindMatrices + byteLengthInverseBindMatrices; i += 4 )
				inverseBindMatricesData.Push(reinterpret_cast<float &>(buffer[i]));

			mat4 inverseBindMatrix(0.0f);
			for ( unsigned int n = 0; n < joints.value.array->GetSize(); ++n ) {
				for ( unsigned int g = 0; g < 4; ++g )
					for ( unsigned int j = 0; j < 4; ++j )
						inverseBindMatrix[g][j] = inverseBindMatricesData[n * 16 + g * 4 + j];

				inverseBindMatrixSet.Push(inverseBindMatrix);
			}

			unsigned int joints_index = (*gltf)["meshes"][0]["primitives"][0]["attributes"]["JOINTS_0"].value.iNumber;
			unsigned int joints_buffer_view_index = (*gltf)["accessors"][joints_index]["bufferView"].value.iNumber;
			unsigned int joints_byte_length = (*gltf)["bufferViews"][joints_buffer_view_index]["byteLength"].value.iNumber;
			unsigned int joints_byte_offset = (*gltf)["bufferViews"][joints_buffer_view_index]["byteOffset"].value.iNumber;

			for ( unsigned int i = joints_byte_offset; i < joints_byte_offset + joints_byte_length; ++i )
				jointsIndices.Push(reinterpret_cast<char &>(buffer[i]));

			unsigned int weights_index = (*gltf)["meshes"][0]["primitives"][0]["attributes"]["WEIGHTS_0"].value.iNumber;
			unsigned int weights_buffer_view_index = (*gltf)["accessors"][weights_index]["bufferView"].value.iNumber;
			unsigned int weights_byte_length = (*gltf)["bufferViews"][weights_buffer_view_index]["byteLength"].value.iNumber;
			unsigned int weights_byte_offset = (*gltf)["bufferViews"][weights_buffer_view_index]["byteOffset"].value.iNumber;

			for ( unsigned int i = weights_byte_offset; i < weights_byte_offset + weights_byte_length; i += 4 )
				weightsContainer.Push(reinterpret_cast<float &>(buffer[i]));
		} else {
			noAnimations = true;
		}

		core::vector<Core::JsonValue> animations = Search("animations");

		if ( animations.GetSize() > 0 ) {
			core::vector<Core::JsonValue> samplerIndices;
			core::vector<Core::JsonValue> targetNodes;
			core::vector<Core::JsonValue> targetPaths;
			Core::JsonValue channels = (*gltf)["animations"][0]["channels"];
			for ( unsigned int i = 0; i < channels.value.array->GetSize(); ++i )
				samplerIndices.Push(channels[i]["sampler"]);

			for ( unsigned int i = 0; i < channels.value.array->GetSize(); ++i )
				targetNodes.Push(channels[i]["target"]["node"]);

			for ( unsigned int i = 0; i < channels.value.array->GetSize(); ++i )
				targetPaths.Push(channels[i]["target"]["path"]);

			core::vector<unsigned int> translationSamplerIndices;
			core::vector<unsigned int> rotationSamplerIndices;
			core::vector<unsigned int> scaleSamplerIndices;
			for ( unsigned int i = 0; i < samplerIndices.GetSize(); ++i ) {
				if ( *targetPaths[i].value.string == "translation" ) {
					translationSamplerIndices.Push(samplerIndices[i].value.iNumber);
				} else if ( *targetPaths[i].value.string == "rotation" ) {
					rotationSamplerIndices.Push(samplerIndices[i].value.iNumber);
				} else if ( *targetPaths[i].value.string == "scale" ) {
					scaleSamplerIndices.Push(samplerIndices[i].value.iNumber);
				}
			}

			Core::JsonValue samplers = (*gltf)["animations"][0]["samplers"];
				
			core::vector<unsigned int> translationInputs;
			core::vector<unsigned int> translationOutputs;
				
			for ( unsigned int i = 0; i < translationSamplerIndices.GetSize(); ++i)
				translationInputs.Push(samplers[translationSamplerIndices[i]]["input"].value.iNumber);

			for ( unsigned int i = 0; i < translationSamplerIndices.GetSize(); ++i)
				translationOutputs.Push(samplers[translationSamplerIndices[i]]["output"].value.iNumber);

			core::vector<core::vector<float>> frameInputsTranslation;
			for ( unsigned int i = 0; i < translationInputs.GetSize(); ++i) {
				unsigned int frameBufferViewIndex =
					(*gltf)["accessors"][translationInputs[i]]["bufferView"].value.iNumber;
				unsigned int frameByteLength      =
					(*gltf)["bufferViews"][frameBufferViewIndex]["byteLength"].value.iNumber;
				unsigned int frameByteOffset      =
					(*gltf)["bufferViews"][frameBufferViewIndex]["byteOffset"].value.iNumber;

				core::vector<float> temp;
				for ( unsigned int i = frameByteOffset; i < frameByteOffset + frameByteLength; i += 4 )
					temp.Push(reinterpret_cast<float &>(buffer[i]));

				frameInputsTranslation.Push(temp);
			}

			core::vector<core::vector<float>> translations;
			for ( unsigned int i = 0; i < translationOutputs.GetSize(); ++i) {
				unsigned int outputBufferViewIndex =
					(*gltf)["accessors"][translationOutputs[i]]["bufferView"].value.iNumber;
				unsigned int outputByteLength      =
					(*gltf)["bufferViews"][outputBufferViewIndex]["byteLength"].value.iNumber;
				unsigned int outputByteOffset      =
					(*gltf)["bufferViews"][outputBufferViewIndex]["byteOffset"].value.iNumber;

				core::vector<float> temp;
				for ( unsigned int i = outputByteOffset; i < outputByteOffset + outputByteLength; i += 4 )
					temp.Push(reinterpret_cast<float &>(buffer[i]));

				translations.Push(temp);
			}

			core::vector<unsigned int> rotationInputs;
			core::vector<unsigned int> rotationOutputs;
				
			for ( unsigned int i = 0; i < rotationSamplerIndices.GetSize(); ++i)
				rotationInputs.Push(samplers[rotationSamplerIndices[i]]["input"].value.iNumber);

			for ( unsigned int i = 0; i < rotationSamplerIndices.GetSize(); ++i)
				rotationOutputs.Push(samplers[rotationSamplerIndices[i]]["output"].value.iNumber);

			core::vector<core::vector<float>> frameInputsRotation;
			for ( unsigned int i = 0; i < rotationInputs.GetSize(); ++i) {
				unsigned int frameBufferViewIndex =
					(*gltf)["accessors"][rotationInputs[i]]["bufferView"].value.iNumber;
				unsigned int frameByteLength      =
					(*gltf)["bufferViews"][frameBufferViewIndex]["byteLength"].value.iNumber;
				unsigned int frameByteOffset      =
					(*gltf)["bufferViews"][frameBufferViewIndex]["byteOffset"].value.iNumber;

				core::vector<float> temp;
				for ( unsigned int i = frameByteOffset; i < frameByteOffset + frameByteLength; i += 4 )
					temp.Push(reinterpret_cast<float &>(buffer[i]));

				frameInputsRotation.Push(temp);
			}

			core::vector<core::vector<float>> rotations;
			for ( unsigned int i = 0; i < rotationOutputs.GetSize(); ++i) {
				unsigned int outputBufferViewIndex =
					(*gltf)["accessors"][rotationOutputs[i]]["bufferView"].value.iNumber;
				unsigned int outputByteLength      =
					(*gltf)["bufferViews"][outputBufferViewIndex]["byteLength"].value.iNumber;
				unsigned int outputByteOffset      =
					(*gltf)["bufferViews"][outputBufferViewIndex]["byteOffset"].value.iNumber;

				core::vector<float> temp;
				for ( unsigned int i = outputByteOffset; i < outputByteOffset + outputByteLength; i += 4 )
					temp.Push(reinterpret_cast<float &>(buffer[i]));

				rotations.Push(temp);
			}
			
			core::vector<unsigned int> scaleInputs;
			core::vector<unsigned int> scaleOutputs;
				
			for ( unsigned int i = 0; i < scaleSamplerIndices.GetSize(); ++i)
				scaleInputs.Push(samplers[scaleSamplerIndices[i]]["input"].value.iNumber);

			for ( unsigned int i = 0; i < scaleSamplerIndices.GetSize(); ++i)
				scaleOutputs.Push(samplers[scaleSamplerIndices[i]]["output"].value.iNumber);

			core::vector<core::vector<float>> frameInputsScale;
			for ( unsigned int i = 0; i < scaleInputs.GetSize(); ++i) {
				unsigned int frameBufferViewIndex =
					(*gltf)["accessors"][scaleInputs[i]]["bufferView"].value.iNumber;
				unsigned int frameByteLength      =
					(*gltf)["bufferViews"][frameBufferViewIndex]["byteLength"].value.iNumber;
				unsigned int frameByteOffset      =
					(*gltf)["bufferViews"][frameBufferViewIndex]["byteOffset"].value.iNumber;

				core::vector<float> temp;
				for ( unsigned int i = frameByteOffset; i < frameByteOffset + frameByteLength; i += 4 )
					temp.Push(reinterpret_cast<float &>(buffer[i]));

				frameInputsScale.Push(temp);
			}

			core::vector<core::vector<float>> scales;
			for ( unsigned int i = 0; i < scaleOutputs.GetSize(); ++i) {
				unsigned int outputBufferViewIndex =
					(*gltf)["accessors"][scaleOutputs[i]]["bufferView"].value.iNumber;
				unsigned int outputByteLength      =
					(*gltf)["bufferViews"][outputBufferViewIndex]["byteLength"].value.iNumber;
				unsigned int outputByteOffset      =
					(*gltf)["bufferViews"][outputBufferViewIndex]["byteOffset"].value.iNumber;

				core::vector<float> temp;
				for ( unsigned int i = outputByteOffset; i < outputByteOffset + outputByteLength; i += 4 )
					temp.Push(reinterpret_cast<float &>(buffer[i]));

				scales.Push(temp);
			}

			/// Searching for parent joins WITH GOAT GOTO OPERATOR!!!
			core::vector<int> parent_joins;
			for ( unsigned int s = 0; s < joints.value.array->GetSize(); ++s ) {
				int current_joint = (*joints.value.array)[s].value.iNumber;

				for ( unsigned w = 0; w < children.GetSize(); ++w ) {
					for ( unsigned q = 0; q < children[w].GetSize(); ++q ) {
						if ( children[w][q] == current_joint )
							goto most_scary_operator_of_all_time;                                        ///< Yes. This is what we all deserve
					}
				}
				parent_joins.Push(current_joint);
				
			  most_scary_operator_of_all_time:                                                           ///< Not so scary at all. Am i right?
				continue;
			}

			frames = frameInputsTranslation[0];
			core::vector<core::vector<mat4>> jointMatricesAccumulator;        ///< Delete this sheet!

			core::vector<core::vector<unsigned int>> joints_bones;
			for ( unsigned int w = 0; w < parent_joins.GetSize(); ++w ) {
				core::vector<core::vector<unsigned int>> nodes_bones;
				unsigned int currentNode = parent_joins[w];
				core::stack<u32> node_stack;
				node_stack.push(currentNode);

				core::stack<u32> deepness_stack;
				traversalBones(children, joints, node_stack, deepness_stack, nodes_bones);

				for ( unsigned int e = 0; e < nodes_bones.GetSize(); ++e ) {
					joints_bones.Push(nodes_bones[e]);
				}
			}

			for ( unsigned int j = 0; j < translations.GetSize(); ++j ) {
				core::vector<float> boneAllFrameTranslations = translations[j];
				core::vector<float> boneAllFrameRotations    = rotations[j];
				core::vector<float> boneAllFrameScales;
				if ( scales.GetSize() > 0 )
					boneAllFrameScales       = scales[j];
				core::vector<mat4>  globalAllFrameNodeMatrix;
				core::vector<mat4>  globalAllFrameNodeMatrixAccumulator;
				for ( unsigned int i = 0; i < frameInputsTranslation[0].GetSize(); ++i ) {
					mat4 frameTranslation(1.0f);
					mat4 frameScale(1.0f);
					for ( unsigned int q = 0; q < 3; ++q ) {
						frameTranslation[3][q] = boneAllFrameTranslations[i * 3 + q];
						if ( scales.GetSize() > 0 )
						frameScale[q][q]       = boneAllFrameScales[i * 3 + q];
					}

					Quaternion frameRotationQuaternion;
					mat4 frameRotation(1.0f);
					frameRotationQuaternion.x = boneAllFrameRotations[i * 4];
					frameRotationQuaternion.y = boneAllFrameRotations[i * 4 + 1];
					frameRotationQuaternion.z = boneAllFrameRotations[i * 4 + 2];
					frameRotationQuaternion.w = boneAllFrameRotations[i * 4 + 3];

					frameRotation = rotateQuaternion<float, 4>(frameRotationQuaternion);
					frameRotation.SelfTensorTranspose();

					mat4 globalTransformNodeMatrix = frameScale * frameRotation * frameTranslation;
					globalAllFrameNodeMatrixAccumulator.Push(globalTransformNodeMatrix);
				}
				jointMatricesAccumulator.Push(globalAllFrameNodeMatrixAccumulator);
			}
			
			for ( unsigned int j = 0; j < translations.GetSize(); ++j ) {
				core::vector<mat4>  globalAllFrameNodeMatrix;
				
				for ( unsigned int i = 0; i < frameInputsTranslation[0].GetSize(); ++i ) {
					mat4 rootTransform(1.0f);
					for ( unsigned int b = 0; b < joints_bones[j].GetSize() - 1; ++b ) {
						rootTransform = jointMatricesAccumulator[joints_bones[j][b]][i] * rootTransform;
					}
					globalAllFrameNodeMatrix.Push(inverseBindMatrixSet[j] * jointMatricesAccumulator[j][i] * rootTransform);
				}
				jointMatrices.Push(globalAllFrameNodeMatrix);
			}
			int maximumJoints     = 6;
			int unitMatricesSize = maximumJoints - jointMatrices.GetSize();

			if ( unitMatricesSize > 0 ) {
				for ( int i = 0; i < unitMatricesSize; ++i) {
					core::vector<mat4>  globalAllFrameNodeMatrix;
					for ( unsigned int j = 0; j < frameInputsTranslation[0].GetSize(); ++j ) {
						mat4 unitMatrix(1.0f);
						globalAllFrameNodeMatrix.Push(unitMatrix);
					}

					jointMatrices.Push(globalAllFrameNodeMatrix);
				}
			}
		}

		jointMatricesPerMesh = jointMatrices;

		for ( uint32_t i = 0; i < indices.GetSize(); ++i ) {
			aIndices_.push_back(i);

			unsigned int index = indices[i] * 3;
			if ( index + 2 < vertices_position.GetSize() ) {
				vec3 position = { vertices_position[index],
					vertices_position[index + 1],
					vertices_position[index + 2] };

				aVertexes_.push_back(position[0]);
				aVertexes_.push_back(position[1]);
				aVertexes_.push_back(position[2]);
			}

			if ( index + 2 < normals.GetSize() ) {
				vec3 normal = { normals[index],
					normals[index + 1],
					normals[index + 2] };

				aVertexes_.push_back(normal[0]);
				aVertexes_.push_back(normal[1]);
				aVertexes_.push_back(normal[2]);
			}

			index = indices[i] * 2;
			if ( index + 1 < texture_coordinates.GetSize() ) {
				aVertexes_.push_back(texture_coordinates[index]);
				aVertexes_.push_back(texture_coordinates[index + 1]);
			}

			index = indices[i] * 4;
			if ( index + 3 < jointsIndices.GetSize() ) {
				aVertexes_.push_back(jointsIndices[index]);
				aVertexes_.push_back(jointsIndices[index + 1]);
				aVertexes_.push_back(jointsIndices[index + 2]);
				aVertexes_.push_back(jointsIndices[index + 3]);
			}

			if ( index + 3 < weightsContainer.GetSize() ) {
				aVertexes_.push_back(weightsContainer[index]);
				aVertexes_.push_back(weightsContainer[index + 1]);
				aVertexes_.push_back(weightsContainer[index + 2]);
				aVertexes_.push_back(weightsContainer[index + 3]);
			}
		}

		delete [] buffer;
		buffer = nullptr;
	}

	void CJsonParser::traversalBones( core::vector<core::vector<int>> children,
									  Core::JsonValue joints,
									  core::stack<u32> node_stack,
									  core::stack<u32> deepness_stack,
									  core::vector<core::vector<u32>>& result ) {
		u32 topJointIndex = 0;
		if ( !node_stack.empty() ) {
			topJointIndex = getJointIndex(joints, node_stack.top());
		}
		
		if ( node_stack.size() > deepness_stack.size() ) {
			u32 firstChild = 0;
			deepness_stack.push(firstChild);
		}

		if ( deepness_stack.empty() )
			return;

		u32 nextNodeIndex = 0;
		if ( !children[topJointIndex].empty() ) {
			if ( deepness_stack.top() > 0 && deepness_stack.top() == children[topJointIndex].GetSize() ) {
				deepness_stack.pop();
				node_stack.pop();
				traversalBones( children, joints, node_stack, deepness_stack, result );
				return;
			}

			if ( deepness_stack.top() > 0 && deepness_stack.top() < children[topJointIndex].GetSize() ) {
				nextNodeIndex = children[topJointIndex][deepness_stack.top()];
				node_stack.push(nextNodeIndex);

				core::vector<u32> current_node_indices;
				for ( u32 i = 0; i < node_stack.size(); ++i ) {
					u32 currentJoinIndex = getJointIndex(joints, node_stack[i]);
					current_node_indices.Push(currentJoinIndex);
				}

				++deepness_stack.top();
				traversalBones( children, joints, node_stack, deepness_stack, result );
				return;
			} else {
				core::vector<u32> current_node_indices;
				for ( u32 i = 0; i < node_stack.size(); ++i ) {
					u32 currentJoinIndex = getJointIndex(joints, node_stack[i]);
					current_node_indices.Push(currentJoinIndex);
				}
		
				result.Push(current_node_indices);
			
				nextNodeIndex = children[topJointIndex][deepness_stack.top()];
				node_stack.push(nextNodeIndex);
				++deepness_stack.top();
				traversalBones( children, joints, node_stack, deepness_stack, result );
				return;
			}
		} else {
			core::vector<u32> current_node_indices;
			for ( u32 i = 0; i < node_stack.size(); ++i ) {
				u32 currentJoinIndex = getJointIndex(joints, node_stack[i]);
				current_node_indices.Push(currentJoinIndex);
			}

			result.Push(current_node_indices);
			
			deepness_stack.pop();
			node_stack.pop();
			traversalBones( children, joints, node_stack, deepness_stack, result );
			return;
		}
	}

	core::vector<core::vector<unsigned int>> CJsonParser::makeRenderJointsIndices(core::vector<core::vector<unsigned int>>& input) {
		core::vector<core::vector<unsigned int>> result;

		bool accumulatorFlag = false;
		bool innerFlag  = false;
		unsigned int accumulator = input[0][0];
		for ( unsigned int i = 0; i < input.GetSize(); ++i ) {
			for ( unsigned int j = 0; j < input[i].GetSize(); ++j ) {
				core::vector<unsigned int> inner;
				for ( unsigned int v = 0; v < j + 1; ++v ) {
					if ( input[i][j] == accumulator && accumulatorFlag ) {
						innerFlag = false;
						continue;
					} else {
						innerFlag = true;
						inner.Push(input[i][v]);

						if ( accumulatorFlag == false )
							accumulatorFlag = true;
					}
				}

				if ( innerFlag )
					result.Push(inner);
			}
		}

		return result;
	}

	bool CJsonParser::containsElemnt(core::vector<core::vector<unsigned int>> container, unsigned int element) {
		bool flag = false;
		
		for ( unsigned int i = 0; i < container.GetSize(); ++i ) {
			for ( unsigned int j = 0; j < container[i].GetSize(); ++j ) {
				if ( container[i][j] == element )
					return true;
			}
		}

		return flag;
	}
	
	u32 CJsonParser::getJointIndex(Core::JsonValue joints, i32 searchingIndex) {
		for ( unsigned int i = 0; i < joints.value.array->GetSize(); ++i ) {
			int currentJointIndex = (*joints.value.array)[i].value.iNumber;

			if ( currentJointIndex == searchingIndex )
				return i;
		}

		return -1;
	}

	CJsonParser::~CJsonParser() {
		delete root_;
	}
}
