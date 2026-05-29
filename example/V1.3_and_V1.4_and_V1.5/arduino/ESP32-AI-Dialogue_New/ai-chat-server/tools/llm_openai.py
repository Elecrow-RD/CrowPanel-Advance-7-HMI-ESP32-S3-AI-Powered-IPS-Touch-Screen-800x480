import asyncio

from openai import OpenAI

from .config import CLIENTS, HISTORY
from openai import AsyncOpenAI
from .tts import test_submit

# ___________這是deepseek接口________________________________________________________________________
# client = AsyncOpenAI(
#     api_key="sk-706b6e8c0c23468ab61d0797ce0ea350",
#     # Replace MOONSHOT_API_KEY here with the API Key you applied for from the OpenAI platform
#     base_url="https://api.deepseek.com"  # Replace the URL address
# )
# model="deepseek-chat"


# ____________這是kimi接口_______________________________________________________________________
# client = AsyncOpenAI(
#     api_key="sk-SXStCWErkjE7vGJqXHlInjarLIYk4ZTkmrbNOV1oqgsUSedy",
#     # Replace MOONSHOT_API_KEY here with the API Key you applied for from the OpenAI platform
#     base_url= "https://api.moonshot.cn/v1" # Replace the URL address
# )
# model="moonshot-v1-8k"


# ____________這是零一萬物接口_______________________________________________________________________
# client = AsyncOpenAI(
#     api_key="cb2883a23ed240409cbc7a3b66b005a6",
#     # Replace MOONSHOT_API_KEY here with the API Key you applied for from the OpenAI platform
#     base_url="https://api.lingyiwanwu.com/v1"  # Replace the URL address
# )
# model = "yi-lightning"


# ____________This is the OpenAI interface._______________________________________________________________________
client = AsyncOpenAI(
    api_key="sk-proj--xxxxxxxxx",  # your api key
    base_url="https://api.openai.com/v1"
)
model = "gpt-4o-mini"



async def chat(input, mac_address, sentence="", all_sentence=""):
    # global HISTORY
    # We construct the user's latest question into a message (role=user) and add it to the end of the messages list.

    HISTORY.append({
        "role": "user",
        "content": input,
    })

    # print("The sent historical records:", HISTORY)
    # Have a conversation with the OpenAI large model using the messages.
    completion = await client.chat.completions.create(
        model=model,
        messages=HISTORY,
        temperature=0.3,
        stream=True
    )

    # Through the API, we get the reply message (role=assistant) from the OpenAI large model.
    count = 1

    async for chunk in completion:

        choices_obj = chunk.choices[0]
        content = choices_obj.delta.content
        # print(chunk)
        finish_reason = choices_obj.finish_reason

        if not finish_reason:
            if not content:
                continue
            all_sentence += content
            sentence += content



            def split_sentence(s, max_len):
                for i in range(min(max_len, len(s)), 0, -1):
                    if s[i - 1] in ['。', '！', '？', '，', '.', '!', '?', ',']:
                        return s[:i], s[i:]
                return s[:max_len], s[max_len:]

            while len(sentence) > 100:
                part, sentence = split_sentence(sentence, 100)
                await CLIENTS[mac_address].send(f"AI:{part}")
                await test_submit(part, mac_address)

                count += 1



            if content in ['。', '！', '？', '，', '.', '!', '?', ','] and (len(sentence) >= 6** count or len(sentence) >= 100):
                # Run TTS in pure streaming mode.
                # print(f"Sentence {str(count)}: {sentence}")
                # print(sentence)
                print("---The items that need to be sent:",sentence)

                await CLIENTS[mac_address].send(f"AI:{sentence}")

                await test_submit(sentence, mac_address)
                count += 1
                sentence = ""

        else:
            print("---The last sentence that needs to be sent:", sentence)
            sentence1=sentence.replace(" ","")
            if sentence1!="":
                await CLIENTS[mac_address].send(f"AI:{sentence}")

                await test_submit(sentence, mac_address)
            sentence = ""
            print("---Total:", all_sentence)
            # await CLIENTS[mac_address].send("clear_screen")
            await CLIENTS[mac_address].send("finish_tts")
    # To enable the OpenAI large model to have a complete memory, we must also add the message returned by the OpenAI large model to the HISTORY list.
    HISTORY.append({
        "role": "assistant",
        "content": all_sentence
    })


    # return assistant_message.content
